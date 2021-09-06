/*
 * Device
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "address_space.h"
#include "interrupt.h"

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

size_t
flash_add_file(device_t *dev, const char *filename, size_t base);

device_t *
rom_init(address_space *parent_as);

void
rom_add_file(device_t *dev, const char *filename, size_t base);

device_t *
ram_init(address_space *parent_as);

device_t *
uart_init(address_space *parent_as, uint32_t irq_num);

device_t *
virtio_mmio_init(address_space *parent_as, uint64_t start, uint64_t end);

void
virtio_set_backend(device_t *dev, void *backend);

void
plic_signal(uint32_t id);

uint32_t
plic_interrupt(void);

intr_type_t
clint_interrupt(void);

#endif /* DEVICE_H */
