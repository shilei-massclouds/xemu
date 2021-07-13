/*
 * VIRTIO_MMIO
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"

typedef struct _virtio_mmio_t
{
    device_t dev;
} virtio_mmio_t;


static uint64_t
virtio_mmio_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
    return 0;
}

static uint64_t
virtio_mmio_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
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

    register_address_space(parent_as, &(virtio_mmio->dev.as));

    return (device_t *) virtio_mmio;
}
