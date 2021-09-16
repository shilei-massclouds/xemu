/*
 * FLASH
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

#include "util.h"
#include "address_space.h"
#include "device.h"
#include "elf.h"
#include "module.h"

#define FLASH_ADDRESS_SPACE_START 0x0000000020000000UL
#define FLASH_ADDRESS_SPACE_END   0x0000000023FFFFFFUL
#define FLASH_ADDRESS_SPACE_SIZE  \
    (FLASH_ADDRESS_SPACE_END - FLASH_ADDRESS_SPACE_START + 1)

typedef struct _flash_t
{
    device_t dev;

    uint8_t *mem_ptr;
    size_t mem_size;
} flash_t;

static uint8_t *
_flash_ptr(void *dev, uint64_t addr, size_t size)
{
    flash_t *flash = (flash_t *) dev;
    if (addr + size > FLASH_ADDRESS_SPACE_SIZE) {
        panic("%s: 0x%llx out of limit 0x%llx\n",
              __func__, addr, FLASH_ADDRESS_SPACE_SIZE);
    }

    if (addr + size > flash->mem_size)
        return NULL;

    return (flash->mem_ptr + addr);
}

static uint64_t
flash_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint8_t *ptr = _flash_ptr(dev, addr, size);
    if (ptr == NULL)
        return 0;

    if (!IN_SAME_PAGE(addr, size))
        panic("%s: out of page boundary 0x%lx (0x%lx)\n",
              __func__, addr, size);

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
flash_write(void *dev, uint64_t addr, uint64_t data, size_t size,
            params_t params)
{
    uint64_t ret = 0;
    uint8_t *ptr = _flash_ptr(dev, addr, size);

    if (!ptr || !IN_SAME_PAGE(addr, size))
        panic("%s: out of page boundary 0x%lx (0x%lx)\n",
              __func__, addr, size);

    switch (size)
    {
    case 8:
        ret = *((uint64_t *)ptr);
        *((uint64_t *)ptr) = (uint64_t)data;
        break;
    case 4:
        ret = *((uint32_t *)ptr);
        *((uint32_t *)ptr) = (uint32_t)data;
        break;
    case 2:
        ret = *((uint16_t *)ptr);
        *((uint16_t *)ptr) = (uint16_t)data;
        break;
    case 1:
        ret = *((uint8_t *)ptr);
        *((uint8_t *)ptr) = (uint8_t)data;
        break;
    default:
        panic("%s: bad size %d\n", __func__, size);
    }

    return ret;
}

device_t *
flash_init(address_space *parent_as)
{
    flash_t *flash;

    flash = calloc(1, sizeof(flash_t));
    flash->dev.name = "flash";

    flash->mem_ptr = NULL;
    flash->mem_size = 0;

    init_address_space(&(flash->dev.as),
                       FLASH_ADDRESS_SPACE_START,
                       FLASH_ADDRESS_SPACE_END);

    flash->dev.as.ops.read_op = flash_read;
    flash->dev.as.ops.write_op = flash_write;

    flash->dev.as.device = flash;

    register_address_space(parent_as, &(flash->dev.as));

    return (device_t *) flash;
}

size_t
flash_add_file(device_t *dev, const char *filename, size_t base)
{
    uint8_t *ptr;
    size_t size;
    struct stat info;
    flash_t *flash = (flash_t *) dev;
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL || fstat(fileno(fp), &info) < 0)
        panic("%s: bad filename %s\n", __func__, filename);

    if (base < flash->mem_size)
        panic("%s: bad base %x\n", __func__, base);

    size = ROUND_UP((base + (size_t)info.st_size), 8);

    if (size > FLASH_ADDRESS_SPACE_SIZE)
        panic("%s: bad size %x\n", __func__, size);

    ptr = calloc(1, size);
    if (ptr == NULL)
        panic("%s: alloc memory failed!\n", __func__);

    if (flash->mem_size > 0) {
        memcpy(ptr, flash->mem_ptr, flash->mem_size);
        free(flash->mem_ptr);
        flash->mem_ptr = NULL;
    }

    if (fread(ptr + base, 1, (size_t)info.st_size, fp) != info.st_size)
        panic("%s: read file failed!\n", __func__);

    flash->mem_size = size;
    flash->mem_ptr = ptr;

    fclose(fp);

    if (elf64_hdr_check(ptr + base))
        elf64_hdr_set_length(ptr + base, (uint64_t)info.st_size);

    printf("%s: add file %s(%lx:%lx)\n",
           __func__, filename, base, flash->mem_size);

    return size;
}

void
flash_load_modules(device_t *dev)
{
    module *mod;
    list_head *mod_list;
    size_t base = 0x100;

    mod_list = sort_modules();

    base = flash_add_file(dev, "image/startup.bin", base);

    list_for_each_entry(mod, mod_list, list) {
        char filename[256] = {0};
        sprintf(filename, "%s%s", KMODULE_DIR, mod->name);
        base = flash_add_file(dev, filename, base);
    }

    clear_modules();
}
