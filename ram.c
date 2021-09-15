/*
 * RAM
 */

#include <malloc.h>

#include "device.h"
#include "address_space.h"
#include "util.h"

#define PD_NUM  1024
#define PT_NUM  1024

#define RAM_ADDRESS_SPACE_START 0x0000000080000000
#define RAM_ADDRESS_SPACE_END   0x00000000FFFFFFFF

typedef struct _ram_t
{
    device_t dev;

    uint64_t *_pg;
} ram_t;

static uint8_t *
_ram_ptr(void *dev, uint64_t addr)
{
    uint32_t pd_idx;
    uint32_t pt_idx;
    uint64_t *pd;
    uint8_t  *pt;

    ram_t *ram = (ram_t *) dev;

    pd_idx = (uint32_t) (addr >> 22);
    if (ram->_pg[pd_idx] == 0)
        ram->_pg[pd_idx] = (uint64_t) calloc(PT_NUM, sizeof(uint64_t));
    pd = (uint64_t *) ram->_pg[pd_idx];

    pt_idx = (addr >> 12) & 0x3FF;
    if (pd[pt_idx] == 0)
        pd[pt_idx] = (uint64_t) calloc(PAGE_SIZE, sizeof(uint8_t));
    pt = (uint8_t *) pd[pt_idx];

    return pt + (addr & 0xFFF);
}

static uint64_t
_read(uint8_t *ptr, size_t size)
{
    switch (size)
    {
    case 8:
        return *((uint64_t *)ptr);
    case 4:
        return *((uint32_t *)ptr);
    case 2:
        return *((uint16_t *)ptr);
    case 1:
        return *((uint8_t *)ptr);
    }

    panic("%s: bad size %d\n", __func__, size);
    return 0;
}

static uint64_t
ram_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint8_t *ptr;
    size_t offset;
    uint64_t hi = 0;

    if (!IN_SAME_PAGE(addr, size)) {
        printf("WARN! %s: (%lx, %lx) crosses boundary!\n",
               __func__, addr, size);

        ptr = _ram_ptr(dev, (addr + size) & PAGE_MASK);
        offset = (addr + size) & (~PAGE_MASK);
        hi = _read(ptr, offset) << (offset << 3);

        size -= offset;
    }

    ptr = _ram_ptr(dev, addr);
    return _read(ptr, size) | hi;
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

static void
_write(uint8_t *ptr, uint64_t data, size_t size)
{
    switch (size)
    {
    case 8:
        *((uint64_t *)ptr) = data;
        break;
    case 4:
        *((uint32_t *)ptr) = (uint32_t)data;
        break;
    case 2:
        *((uint16_t *)ptr) = (uint16_t)data;
        break;
    case 1:
        *((uint8_t *)ptr) = (uint8_t)data;
        break;
    default:
        panic("%s: bad size %d\n", __func__, size);
    }
}

static uint64_t
ram_write(void *dev, uint64_t addr, uint64_t data, size_t size,
          params_t params)
{
    uint8_t *ptr;
    uint64_t ret = 0;

    if (params != PARAMS_LR_SC)
        ret = ram_read(dev, addr, size, params);

    if (size == 8)
        data = _amo64(ret, data, params);
    else if (size == 4)
        data = _amo32((uint32_t)ret, (uint32_t)data, params);

    if (!IN_SAME_PAGE(addr, size)) {
        size_t offset;
        uint64_t hi;

        printf("WARN! %s: (%lx, %lx) crosses boundary!\n",
               __func__, addr, size);

        offset = (addr + size) & (~PAGE_MASK);
        hi = data >> (offset << 3);
        ptr = _ram_ptr(dev, (addr + size) & PAGE_MASK);
        _write(ptr, hi, offset);

        size -= offset;
    }

    ptr = _ram_ptr(dev, addr);
    _write(ptr, data, size);

    return ret;
}

device_t *
ram_init(address_space *parent_as)
{
    ram_t *ram;

    ram = calloc(1, sizeof(ram_t));
    ram->dev.name = "ram";

    ram->_pg = calloc(PD_NUM, sizeof(uint64_t *));

    init_address_space(&(ram->dev.as),
                       RAM_ADDRESS_SPACE_START,
                       RAM_ADDRESS_SPACE_END);

    ram->dev.as.ops.read_op = ram_read;
    ram->dev.as.ops.write_op = ram_write;

    ram->dev.as.device = ram;

    register_address_space(parent_as, &(ram->dev.as));

    return (device_t *) ram;
}
