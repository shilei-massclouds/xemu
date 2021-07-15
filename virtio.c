/*
 * VIRTIO
 */

#include <stddef.h>
#include <stdio.h>
#include <string.h>
#include <malloc.h>

#include "virtio.h"
#include "address_space.h"

static inline uint16_t
vring_avail_ring(vqueue_t *vq, int i)
{
    vring_t *vring = (vring_t *)vq;
    uint64_t pa = vring->avail + offsetof(vring_avail_t, ring[i]);
    return read_nommu(NULL, pa, 2, 0);
}

static int
vqueue_get_head(vqueue_t *vq, uint32_t idx, uint32_t *head)
{
    /* Grab the next descriptor number they're advertising, and increment
     * the index we've seen. */
    *head = vring_avail_ring(vq, idx % vq->vring.num);

    /* If their number is silly, that's a fatal mistake. */
    if (*head >= vq->vring.num)
        panic("bad index %u\n", *head);

    return 0;
}

static void
vring_read(uint64_t addr, uint8_t *data, size_t size)
{
    uint64_t dword;
    uint8_t byte;

    while (size >= 8) {
        dword = read_nommu(NULL, addr, 8, 0);
        memcpy(data, &dword, 8);
        size -= 8;
        addr += 8;
        data += 8;
    }

    while (size) {
        byte = read_nommu(NULL, addr, 1, 0);
        memcpy(data, &byte, 1);
        size--;
        addr++;
        data++;
    }
}

static void
vring_desc_read(vqueue_t *vq, uint32_t idx, vring_desc_t *desc)
{
    vring_t *vring = (vring_t *)vq;
    uint64_t pa = vring->desc + idx * sizeof(vring_desc_t);
    vring_read(pa, (uint8_t *)desc, sizeof(vring_desc_t));
}

vq_item_t *
vring_desc_read_indirect(uint64_t addr, uint32_t len)
{
    uint64_t offset = 0;
    uint32_t iov_num = 0;
    iovec_t iov[VIRTQUEUE_MAX_SIZE];
    vring_desc_t desc;
    vq_item_t *item;

    if ((len == 0) || (len % sizeof(vring_desc_t)))
        panic("bad size %u for indirect table\n", len);

    while (1) {
        vring_read(addr + offset, (uint8_t *)&desc, sizeof(vring_desc_t));

        iov[iov_num].base   = desc.addr;
        iov[iov_num].len    = desc.len;
        iov[iov_num].flags  = (desc.flags & VRING_DESC_F_WRITE) ?
                              IO_VEC_F_WRITE : IO_VEC_F_READ;
        iov_num++;

        if (!(desc.flags & VRING_DESC_F_NEXT))
            break;

        offset = desc.next * sizeof(vring_desc_t);
    }

    item = malloc(sizeof(vq_item_t));
    item->num = iov_num;
    item->iov = malloc(iov_num * sizeof(iovec_t));
    memcpy(item->iov, iov, iov_num * sizeof(iovec_t));

    return item;
}

vq_item_t *
vqueue_pop(vqueue_t *vq)
{
    uint32_t head;
    vring_desc_t desc;

    if (vqueue_get_head(vq, vq->last_avail_idx++, &head) < 0)
        return NULL;

    printf("%s: head %u\n", __func__, head);

    vring_desc_read(vq, head, &desc);

    if (desc.flags & VRING_DESC_F_INDIRECT) {
        return vring_desc_read_indirect(desc.addr, desc.len);
    } else {
        panic("%s: now only support indirect desc table!\n", __func__);
    }

    printf("%s: desc addr(0x%lx) len(%u) flags(0x%x) next(%u)\n",
           __func__, desc.addr, desc.len, desc.flags, desc.next);

    return NULL;
}

void
vring_init(vring_t *vring, uint64_t pfn, uint32_t page_shift)
{
    vring->desc = pfn << page_shift;

    if (!vring->num || !vring->desc || !vring->align) {
        panic("%s: not yet setup\n", __func__);
    }

    vring->avail = vring->desc + vring->num * sizeof(vring_desc_t);

    vring->used = vring_align(vring->avail +
                              offsetof(vring_avail_t, ring[vring->num]),
                              vring->align);
}
