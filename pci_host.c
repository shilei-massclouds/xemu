/*
 * PLIC
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"

#define PLIC_ADDRESS_SPACE_START 0x000000000C000000
#define PLIC_ADDRESS_SPACE_END   0x000000000FFFFFFF

typedef struct _plic_t
{
    device_t dev;
} plic_t;


static uint64_t
plic_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
    return 0;
}

static uint64_t
plic_write(void *dev, uint64_t addr, uint64_t data, size_t size,
           params_t params)
{
    fprintf(stderr, "%s: need to be implemented!\n", __func__);
    return 0;
}

device_t *
plic_init(address_space *parent_as)
{
    plic_t *plic;

    plic = calloc(1, sizeof(plic_t));
    plic->dev.name = "plic";

    init_address_space(&(plic->dev.as),
                       PLIC_ADDRESS_SPACE_START,
                       PLIC_ADDRESS_SPACE_END);

    plic->dev.as.ops.read_op = plic_read;
    plic->dev.as.ops.write_op = plic_write;

    plic->dev.as.device = plic;

    register_address_space(parent_as, &(plic->dev.as));

    return (device_t *) plic;
}

/*
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>

#include "util.h"
*/
