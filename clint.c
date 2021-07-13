/*
 * CLINT
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"

#define CLINT_ADDRESS_SPACE_START 0x0000000002000000
#define CLINT_ADDRESS_SPACE_END   0x000000000200FFFF

typedef struct _clint_t
{
    device_t dev;
} clint_t;


static uint64_t
clint_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
    return 0;
}

static uint64_t
clint_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
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
