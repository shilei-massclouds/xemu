/*
 * VIRTIO_MMIO
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"
#include "virtio.h"
#include "util.h"

typedef struct _virtio_mmio_t
{
    device_t dev;

    virtio_dev_t *backend;
} virtio_mmio_t;


static uint64_t
virtio_mmio_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    virtio_mmio_t *vdev = (virtio_mmio_t *) dev;

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

    default:
        panic("%s: bad reg addr 0x%lx\n", __func__, addr);
    }

    printf("%s: addr(0x%lx) size(%ld) need to be implemented!\n",
            __func__, addr, size);
    return 0;
}

static uint64_t
virtio_mmio_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    if (size != 4)
        panic("%s: bad size %ld\n", __func__, size);

    printf("%s: addr(0x%lx) size(%ld) need to be implemented!\n",
            __func__, addr, size);
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

void
virtio_set_backend(device_t *dev, void *backend)
{
    virtio_mmio_t *vdev = (virtio_mmio_t *) dev;
    vdev->backend = backend;
}
