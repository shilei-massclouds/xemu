/*
 * Address Space
 */

#include <stdio.h>

#include "address_space.h"
#include "util.h"
#include "mmu.h"

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

static uint64_t
mmu_dummy(uint64_t vaddr, int *except)
{
    return vaddr;
}

void
init_address_space(address_space *as, uint64_t start, uint64_t end)
{
    as->start = start;
    as->end   = end;

    as->ops.read_op = read_dummy;
    as->ops.write_op = write_dummy;

    as->device = NULL;
    as->mmu = mmu_dummy;

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
read(address_space *as, uint64_t vaddr, size_t size, params_t params,
     int *except)
{
    address_space *child;
    uint64_t paddr = as->mmu(vaddr, except);

    if (!(size == 1 || size == 2 || size == 4 || size == 8))
        panic("%s: bad size %d\n", __func__, size);

    if (*except)
        return 0;

    child = as->children;
    while (child) {
        if (paddr >= child->start && paddr <= child->end)
            return read(child, paddr - child->start, size, params, except);

        child = child->sibling;
    }

    return as->ops.read_op(as->device, paddr, size, params);
}

uint64_t
write(address_space *as, uint64_t vaddr, size_t size, uint64_t data,
      params_t params, int *except)
{
    address_space *child;
    uint64_t paddr = as->mmu(vaddr, except);

    if (!(size == 1 || size == 2 || size == 4 || size == 8))
        panic("%s: bad size %d\n", __func__, size);

    if (*except)
        return 0;

    child = as->children;
    while (child) {
        if (paddr >= child->start && paddr <= child->end) {
            return write(child, paddr - child->start, size, data,
                         params, except);
        }

        child = child->sibling;
    }

    return as->ops.write_op(as->device, paddr, data, size, params);
}
