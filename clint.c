/*
 * CLINT
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"
#include "util.h"

#define CLINT_ADDRESS_SPACE_START 0x0000000002000000
#define CLINT_ADDRESS_SPACE_END   0x000000000200FFFF

#define CLINT_MSIP      0x0000
#define CLINT_MTIMECMP  0x4000
#define CLINT_MTIME     0xBFF8

typedef struct _clint_t
{
    device_t dev;

    uint32_t msip;

    uint32_t mtime;
    uint32_t mtimecmp;
} clint_t;


static uint64_t
clint_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    //clint_t *clint = (clint_t *) dev;

    switch (addr)
    {
    default:
        panic("%s: need to be implemented! [0x%lx]: (%u)\n",
              __func__, addr, size);
    }

    return 0;
}

static uint64_t
clint_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    clint_t *clint = (clint_t *) dev;

    switch (addr)
    {
    case CLINT_MSIP:
        clint->msip = (uint32_t) data;
        break;
    case CLINT_MTIMECMP:
        clint->mtimecmp = data;
        break;
    default:
        panic("%s: need to be implemented! [0x%lx]: 0x%lx (%u)\n",
              __func__, addr, data, size);
    }

    return 0;
}

device_t *
clint_init(address_space *parent_as)
{
    clint_t *clint;

    clint = calloc(1, sizeof(clint_t));
    clint->dev.name = "clint";

    init_address_space(&(clint->dev.as),
                       CLINT_ADDRESS_SPACE_START,
                       CLINT_ADDRESS_SPACE_END);

    clint->dev.as.ops.read_op = clint_read;
    clint->dev.as.ops.write_op = clint_write;

    clint->dev.as.device = clint;

    register_address_space(parent_as, &(clint->dev.as));

    return (device_t *) clint;
}
