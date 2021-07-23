/*
 * FW-CFG
 */

#include <malloc.h>

#include "address_space.h"
#include "device.h"
#include "util.h"

#define FW_CFG_ADDRESS_SPACE_START 0x0000000010100000
#define FW_CFG_ADDRESS_SPACE_END   0x0000000010100017

typedef struct _fw_cfg_t
{
    device_t dev;
} fw_cfg_t;


static uint64_t
fw_cfg_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    //fw_cfg_t *fw_cfg = (fw_cfg_t *) dev;

    switch (addr)
    {
    default:
        panic("%s: need to be implemented! [0x%lx]: (%u)\n",
              __func__, addr, size);
    }

    return 0;
}

static uint64_t
fw_cfg_write(void *dev, uint64_t addr, uint64_t data, size_t size,
             params_t params)
{
    //fw_cfg_t *fw_cfg = (fw_cfg_t *) dev;

    switch (addr)
    {
    default:
        panic("%s: need to be implemented! [0x%lx]: (%u)\n",
              __func__, addr, size);
    }

    return 0;
}

device_t *
fw_cfg_init(address_space *parent_as)
{
    fw_cfg_t *fw_cfg;

    fw_cfg = calloc(1, sizeof(fw_cfg_t));
    fw_cfg->dev.name = "fw_cfg";

    init_address_space(&(fw_cfg->dev.as),
                       FW_CFG_ADDRESS_SPACE_START,
                       FW_CFG_ADDRESS_SPACE_END);

    fw_cfg->dev.as.ops.read_op = fw_cfg_read;
    fw_cfg->dev.as.ops.write_op = fw_cfg_write;

    fw_cfg->dev.as.device = fw_cfg;

    register_address_space(parent_as, &(fw_cfg->dev.as));

    return (device_t *) fw_cfg;
}
