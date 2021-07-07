/*
 * AddressSpace
 */

#ifndef ADDRESS_SPACE_H
#define ADDRESS_SPACE_H

#include <stdio.h>
#include <stdint.h>

/* Sv39 for riscv64 */
#define ROOT_ADDRESS_SPACE_START 0x0000000000000000
#define ROOT_ADDRESS_SPACE_END   0x0000007FFFFFFFFF

typedef struct _address_space
{
    uint64_t start;
    uint64_t end;

    struct {
        uint64_t (*read_op)(void *dev, uint64_t addr, size_t size);
        void (*write_op)(void *dev, uint64_t addr, uint64_t data, size_t size);
    } ops;

    void *device;

    struct _address_space *children;
    struct _address_space *sibling;

} address_space;

void init_address_space(address_space *as, uint64_t start, uint64_t end);

void register_address_space(address_space *parent, address_space *child);

uint32_t read32(address_space *as, uint64_t addr);
void write32(address_space *as, uint64_t addr, uint32_t data);

uint64_t read64(address_space *as, uint64_t addr);
void write64(address_space *as, uint64_t addr, uint64_t data);

#endif /* ADDRESS_SAPCE_H */
