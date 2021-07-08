/*
 * Address Space
 */

#include <stdio.h>

#include "address_space.h"
#include "util.h"

static uint64_t
read_dummy(void *dev, uint64_t addr, size_t size)
{
    panic("%s: bad addr(0x%llx)!\n", __func__, addr);
    return 0;
}

static void
write_dummy(void *dev, uint64_t addr, uint64_t data, size_t size)
{
    panic("%s: bad addr(0x%llx)\n", __func__, addr);
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
read64(address_space *as, uint64_t addr)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end)
            return read64(child, addr - child->start);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, addr, 8);
}

void
write64(address_space *as, uint64_t addr, uint64_t data)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end) {
            write64(child, addr - child->start, data);
            return;
        }

        child = child->sibling;
    }

    as->ops.write_op(as->device, addr, data, 8);
}

uint32_t
read32(address_space *as, uint64_t addr)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end)
            return read32(child, addr - child->start);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, addr, 4);
}

void
write32(address_space *as, uint64_t addr, uint32_t data)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end) {
            write32(child, addr - child->start, data);
            return;
        }

        child = child->sibling;
    }

    as->ops.write_op(as->device, addr, data, 4);
}

uint16_t
read16(address_space *as, uint64_t addr)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end)
            return read16(child, addr - child->start);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, addr, 2);
}

void
write16(address_space *as, uint64_t addr, uint16_t data)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end) {
            write16(child, addr - child->start, data);
            return;
        }

        child = child->sibling;
    }

    as->ops.write_op(as->device, addr, data, 2);
}

uint8_t
read8(address_space *as, uint64_t addr)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end)
            return read8(child, addr - child->start);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, addr, 1);
}

void
write8(address_space *as, uint64_t addr, uint8_t data)
{
    address_space *child = as->children;

    while (child) {
        if (addr >= child->start && addr <= child->end) {
            write8(child, addr - child->start, data);
            return;
        }

        child = child->sibling;
    }

    as->ops.write_op(as->device, addr, data, 1);
}
