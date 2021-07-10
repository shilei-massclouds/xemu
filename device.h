/*
 * Device
 */

#ifndef DEVICE_H
#define DEVICE_H

#include "address_space.h"

typedef struct _device
{
    const char *name;
    address_space as;
} device_t;

device_t *
rom_init(address_space *parent_as);

void
rom_add_file(device_t *dev, const char *filename, size_t base);

device_t *
ram_init(address_space *parent_as);

device_t *
uart_init(address_space *parent_as);

#endif /* DEVICE_H */
