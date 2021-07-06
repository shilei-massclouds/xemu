/*
 * Device
 */

#ifndef DEVICE_H
#define DEVICE_H

typedef struct _device
{
    const char *name;
    address_space as;
} device_t;

device_t *
rom_init(address_space *parent_as);

void
rom_add_file(device_t *dev, const char *filename);

#endif /* DEVICE_H */
