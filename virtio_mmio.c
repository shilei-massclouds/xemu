/*
 * VIRTIO_MMIO
 */

#include <malloc.h>

#include "types.h"
#include "address_space.h"
#include "device.h"
#include "virtio.h"
#include "util.h"

typedef struct _virtio_mmio_t
{
    device_t dev;

    uint64_t    host_features;
    bool        host_features_sel;
    uint64_t    guest_features;
    bool        guest_features_sel;
    uint32_t    guest_page_shift;
    uint16_t    queue_sel;
    uint64_t    vring_desc;
    uint32_t    vring_num;
    uint32_t    vring_align;
    uint8_t     status;

    virtio_dev_t *backend;
} virtio_mmio_t;


static uint64_t
virtio_mmio_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint64_t host_features = 0;
    virtio_mmio_t *vdev = (virtio_mmio_t *) dev;

    if (addr >= VIRTIO_MMIO_CONFIG) {
        addr -= VIRTIO_MMIO_CONFIG;

        if (size != 1)
            panic("%s: bad size %ld\n", __func__, size);

        if (vdev->backend == NULL)
            panic("%s: NO backend\n", __func__);

        return vdev->backend->config[addr];
    }

    if (size != 4)
        panic("%s: bad size %ld\n", __func__, size);

    switch (addr)
    {
    case VIRTIO_MMIO_MAGIC_VALUE:
        return VIRT_MAGIC;

    case VIRTIO_MMIO_VERSION:
        return VIRT_VERSION_LEGACY;

    case VIRTIO_MMIO_DEVICE_ID:
        return vdev->backend ? vdev->backend->id : 0;

    case VIRTIO_MMIO_VENDOR_ID:
        return VIRT_VENDOR;

    case VIRTIO_MMIO_DEVICE_FEATURES:
        if (vdev->backend)
            host_features = vdev->backend->get_features(vdev->host_features);

        return vdev->host_features_sel ? 0 : host_features;

    case VIRTIO_MMIO_QUEUE_NUM_MAX:
        return VIRTQUEUE_MAX_SIZE;

    case VIRTIO_MMIO_QUEUE_PFN:
        return vdev->vring_desc >> vdev->guest_page_shift;

    case VIRTIO_MMIO_STATUS:
        return vdev->status;
        break;

    default:
        printf("%s: addr(0x%lx) size(%ld) need to be implemented!\n",
                __func__, addr, size);
        panic("%s: bad reg addr 0x%lx\n", __func__, addr);
    }

    return 0;
}

static uint64_t
virtio_mmio_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    virtio_mmio_t *vdev = (virtio_mmio_t *) dev;

    if (size != 4)
        panic("%s: bad size %ld\n", __func__, size);

    switch (addr)
    {
    case VIRTIO_MMIO_DEVICE_FEATURES_SEL:
        vdev->host_features_sel = !!data;
        break;

    case VIRTIO_MMIO_DRIVER_FEATURES:
        if (!vdev->guest_features_sel)
            vdev->guest_features = data;
        break;

    case VIRTIO_MMIO_DRIVER_FEATURES_SEL:
        vdev->guest_features_sel = !!data;
        break;

    case VIRTIO_MMIO_GUEST_PAGE_SIZE:
        vdev->guest_page_shift = ctz32(data);
        if (vdev->guest_page_shift > 31)
            vdev->guest_page_shift = 0;
        break;

    case VIRTIO_MMIO_QUEUE_SEL:
        if (data < VIRTIO_QUEUE_MAX)
            vdev->queue_sel = data;
        break;

    case VIRTIO_MMIO_QUEUE_PFN:
        if (data)
            vdev->vring_desc = (data << vdev->guest_page_shift);
        else
            vdev->vring_desc = 0;

        break;

    case VIRTIO_MMIO_QUEUE_NUM:
        vdev->vring_num = data;
        break;

    case VIRTIO_MMIO_QUEUE_ALIGN:
        vdev->vring_align = data;
        break;

    case VIRTIO_MMIO_QUEUE_NOTIFY:
        printf("notify!\n");
        break;

    case VIRTIO_MMIO_STATUS:
        vdev->status = data & 0xFF;
        break;

    default:
        printf("%s: addr(0x%lx) size(%ld) need to be implemented!\n",
                __func__, addr, size);
        panic("%s: bad reg addr 0x%lx\n", __func__, addr);
    }

    return 0;
}

device_t *
virtio_mmio_init(address_space *parent_as, uint64_t start, uint64_t end)
{
    virtio_mmio_t *virtio_mmio;

    virtio_mmio = calloc(1, sizeof(virtio_mmio_t));
    virtio_mmio->dev.name = "virtio_mmio";

    init_address_space(&(virtio_mmio->dev.as), start, end);

    virtio_mmio->dev.as.ops.read_op = virtio_mmio_read;
    virtio_mmio->dev.as.ops.write_op = virtio_mmio_write;

    virtio_mmio->dev.as.device = virtio_mmio;

    virtio_mmio->host_features = VIRTIO_COMMON_FEATURES;

    register_address_space(parent_as, &(virtio_mmio->dev.as));

    return (device_t *) virtio_mmio;
}

void
virtio_set_backend(device_t *dev, void *backend)
{
    virtio_mmio_t *vdev = (virtio_mmio_t *) dev;
    vdev->backend = backend;
}
