/*
 * Address Space
 */

#include <stdio.h>
#include <string.h>

#include "util.h"
#include "mmu.h"

address_space root_as;

static uint64_t
read_dummy(void *dev, uint64_t addr, size_t size, params_t params)
{
    panic("%s: bad addr(0x%llx)!\n", __func__, addr);
    return 0;
}

static uint64_t
write_dummy(void *dev, uint64_t addr, uint64_t data, size_t size,
            params_t params)
{
    panic("%s: bad addr(0x%llx)\n", __func__, addr);
    return 0;
}

void
init_address_space(address_space *as, uint64_t start, uint64_t end)
{
    as->start = start;
    as->end   = end;

    as->ops.read_op = read_dummy;
    as->ops.write_op = write_dummy;

    as->device = NULL;

    as->children = NULL;
    as->sibling = NULL;
}

void
register_address_space(address_space *parent, address_space *child)
{
    if (parent->children)
        child->sibling = parent->children;

    parent->children = child;
}

uint64_t
read_nommu(address_space *as, uint64_t addr, size_t size, params_t params)
{
    address_space *child;

    if (!(size == 1 || size == 2 || size == 4 || size == 8))
        panic("%s: bad size %d\n", __func__, size);

    if (as == NULL)
        as = &root_as;

    child = as->children;
    while (child) {
        if (addr >= child->start && addr <= child->end)
            return read_nommu(child, addr - child->start, size, params);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, addr, size, params);
}

uint64_t
read(address_space *as, uint64_t vaddr, size_t size, params_t params,
     int *except)
{
    uint64_t paddr = mmu(as, vaddr, except);
    if (*except)
        return 0;

    return read_nommu(as, paddr, size, params);
}

uint64_t
write_nommu(address_space *as, uint64_t addr, size_t size, uint64_t data,
            params_t params)
{
    address_space *child;

    if (!(size == 1 || size == 2 || size == 4 || size == 8))
        panic("%s: bad size %d\n", __func__, size);

    if (as == NULL)
        as = &root_as;

    child = as->children;
    while (child) {
        if (addr >= child->start && addr <= child->end) {
            return write_nommu(child, addr - child->start, size, data,
                               params);
        }

        child = child->sibling;
    }

    return as->ops.write_op(as->device, addr, data, size, params);
}

uint64_t
write(address_space *as, uint64_t vaddr, size_t size, uint64_t data,
      params_t params, int *except)
{
    uint64_t paddr = mmu(as, vaddr, except);
    if (*except)
        return 0;

    return write_nommu(as, paddr, size, data, params);
}

void
read_blob(uint64_t addr, size_t size, uint8_t *data)
{
    uint64_t dword;
    uint8_t byte;

    if ((addr % 8))
        panic("%s: not align to 8\n", __func__);

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

void
write_blob(uint64_t addr, size_t size, uint8_t *data)
{
    uint64_t dword;
    uint8_t byte;

    if ((addr % 8))
        panic("%s: addr 0x%lx not align to 8\n", __func__, addr);

    while (size >= 8) {
        memcpy(&dword, data, 8);
        write_nommu(NULL, addr, 8, dword, 0);
        size -= 8;
        addr += 8;
        data += 8;
    }

    while (size) {
        byte = *data;
        write_nommu(NULL, addr, 1, byte, 0);
        size--;
        addr++;
        data++;
    }
}
