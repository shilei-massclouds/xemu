/*
 * RAM
 */

#include <malloc.h>

#include "device.h"
#include "address_space.h"
#include "util.h"

#define RAM_ADDRESS_SPACE_START 0x0000000080000000UL
#define RAM_ADDRESS_SPACE_END   0x00000000FFFFFFFFUL

typedef struct _ram_t
{
    device_t dev;

    uint8_t *mem_ptr;
    size_t  mem_size;
} ram_t;

static uint64_t
ram_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint64_t ret = 0;
    ram_t *ram = (ram_t *) dev;

    memcpy(&ret, ram->mem_ptr + addr, size);
    return ret;
}

static uint32_t
_amo32(uint32_t orig, uint32_t data, params_t params)
{
    uint32_t ret = orig;

    switch (params)
    {
    case PARAMS_LR_SC:
        ret = data;
        break;
    case PARAMS_AMO_MIN:
        if ((int32_t)data < (int32_t)orig)
            ret = data;
        break;
    case PARAMS_AMO_MAX:
        if ((int32_t)data > (int32_t)orig)
            ret = data;
        break;
    case PARAMS_AMO_MINU:
        if (data < orig)
            ret = data;
        break;
    case PARAMS_AMO_MAXU:
        if (data > orig)
            ret = data;
        break;
    case PARAMS_AMO_ADD:
        ret = orig + data;
        break;
    case PARAMS_AMO_XOR:
        ret = orig ^ data;
        break;
    case PARAMS_AMO_OR:
        ret = orig | data;
        break;
    case PARAMS_AMO_AND:
        ret = orig & data;
        break;
    case PARAMS_AMO_SWAP:
        ret = data;
        break;

    case PARAMS_NONE:
        ret = data;
        break;
    case PARAMS_LAST:
        break;
    }

    return ret;
}

static uint64_t
_amo64(uint64_t orig, uint64_t data, params_t params)
{
    uint64_t ret = orig;

    switch (params)
    {
    case PARAMS_LR_SC:
        ret = data;
        break;
    case PARAMS_AMO_MIN:
        if ((int64_t)data < (int64_t)orig)
            ret = data;
        break;
    case PARAMS_AMO_MAX:
        if ((int64_t)data > (int64_t)orig)
            ret = data;
        break;
    case PARAMS_AMO_MINU:
        if (data < orig)
            ret = data;
        break;
    case PARAMS_AMO_MAXU:
        if (data > orig)
            ret = data;
        break;
    case PARAMS_AMO_ADD:
        ret = orig + data;
        break;
    case PARAMS_AMO_XOR:
        ret = orig ^ data;
        break;
    case PARAMS_AMO_OR:
        ret = orig | data;
        break;
    case PARAMS_AMO_AND:
        ret = orig & data;
        break;
    case PARAMS_AMO_SWAP:
        ret = data;
        break;

    case PARAMS_NONE:
        ret = data;
        break;
    case PARAMS_LAST:
        break;
    }

    return ret;
}

static uint64_t
ram_write(void *dev, uint64_t addr, uint64_t data, size_t size,
          params_t params)
{
    uint64_t ret = 0;
    ram_t *ram = (ram_t *) dev;

    if (params != PARAMS_LR_SC)
        ret = ram_read(dev, addr, size, params);

    if (size == 8)
        data = _amo64(ret, data, params);
    else if (size == 4)
        data = _amo32((uint32_t)ret, (uint32_t)data, params);

    memcpy(ram->mem_ptr + addr, &data, size);
    return ret;
}

device_t *
ram_init(address_space *parent_as)
{
    ram_t *ram;

    ram = calloc(1, sizeof(ram_t));
    ram->dev.name = "ram";

    ram->mem_size = (RAM_ADDRESS_SPACE_END - RAM_ADDRESS_SPACE_START) + 1;
    ram->mem_ptr = calloc(ram->mem_size, 1);

    init_address_space(&(ram->dev.as),
                       RAM_ADDRESS_SPACE_START,
                       RAM_ADDRESS_SPACE_END);

    ram->dev.as.ops.read_op = ram_read;
    ram->dev.as.ops.write_op = ram_write;

    ram->dev.as.device = ram;

    register_address_space(parent_as, &(ram->dev.as));

    return (device_t *) ram;
}
