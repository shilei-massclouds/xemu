/*
 * Address Space
 */

#include <stdio.h>

#include "address_space.h"
#include "util.h"

static uint64_t
read64_dummy(void *dev, uint64_t addr)
{
    panic("%s: bad addr(0x%llx)!\n", __func__, addr);
    return 0;
}

static void
write64_dummy(void *dev, uint64_t addr, uint64_t data)
{
    panic("%s: bad addr(0x%llx)\n", __func__, addr);
}

void
init_address_space(address_space *as, uint64_t start, uint64_t end)
{
    as->start = start;
    as->end   = end;

    as->ops.read64_op = &read64_dummy;
    as->ops.write64_op = &write64_dummy;

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

    return as->ops.read64_op(as->device, addr);
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
    }

    as->ops.write64_op(as->device, addr, data);
}
