/*
 * VIRTIO-BLK
 */

#include <stdint.h>
#include <malloc.h>

#include "virtio.h"

typedef struct _virtio_blk_t
{
    virtio_dev_t vdev;
} virtio_blk_t;


static uint32_t
virtio_blk_config_readb(virtio_dev_t *dev, uint32_t addr)
{
    printf("%s: need to be implemented!\n", __func__);
    return 0;
}

static void
virtio_blk_config_writeb(virtio_dev_t *dev, uint32_t addr, uint32_t data)
{
    printf("%s: need to be implemented!\n", __func__);
}

virtio_dev_t *
virtio_blk_init()
{
    virtio_blk_t *blk;

    blk = calloc(1, sizeof(virtio_blk_t));
    blk->vdev.id = VIRTIO_ID_BLOCK;
    blk->vdev.config_readb = virtio_blk_config_readb;
    blk->vdev.config_writeb = virtio_blk_config_writeb;

    return (virtio_dev_t *) blk;
}
