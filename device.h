/*
 * Device
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "address_space.h"

typedef struct _device
{
    const char      *name;
    address_space   as;
} device_t;

device_t *
rtc_init(address_space *parent_as);

device_t *
pci_host_init(address_space *parent_as);

device_t *
plic_init(address_space *parent_as);

device_t *
clint_init(address_space *parent_as);

device_t *
flash_init(address_space *parent_as);

void
flash_add_file(device_t *dev, const char *filename, size_t base);

device_t *
rom_init(address_space *parent_as);

void
rom_add_file(device_t *dev, const char *filename, size_t base);

device_t *
ram_init(address_space *parent_as);

device_t *
uart_init(address_space *parent_as);

device_t *
virtio_mmio_init(address_space *parent_as, uint64_t start, uint64_t end);

void
virtio_set_backend(device_t *dev, void *backend);

#endif /* DEVICE_H */
