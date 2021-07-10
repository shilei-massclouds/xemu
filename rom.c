/*
 * ROM
 */

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <sys/stat.h>

#include "util.h"
#include "address_space.h"
#include "device.h"

#define ROM_ADDRESS_SPACE_START 0x0000000000001000
#define ROM_ADDRESS_SPACE_END   0x000000000FFFFFFF

typedef struct _rom_t
{
    device_t dev;

    uint8_t *mem_ptr;
    size_t mem_size;
} rom_t;


static uint8_t *
_rom_ptr(void *dev, uint64_t addr, size_t size)
{
    rom_t *rom = (rom_t *) dev;
    if (addr + size > rom->mem_size) {
        panic("%s: 0x%llx out of limit 0x%lx\n",
              __func__, addr, rom->mem_size);
    }

    return (rom->mem_ptr + addr);
}

static uint64_t
rom_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint8_t *ptr = _rom_ptr(dev, addr, size);

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

device_t *
rom_init(address_space *parent_as)
{
    rom_t *rom;

    rom = calloc(1, sizeof(rom_t));
    rom->dev.name = "rom";

    rom->mem_ptr = NULL;
    rom->mem_size = 0;

    init_address_space(&(rom->dev.as),
                       ROM_ADDRESS_SPACE_START,
                       ROM_ADDRESS_SPACE_END);

    rom->dev.as.ops.read_op = rom_read;

    rom->dev.as.device = rom;

    register_address_space(parent_as, &(rom->dev.as));

    return (device_t *) rom;
}

void
rom_add_file(device_t *dev, const char *filename, size_t base)
{
    uint8_t *ptr;
    size_t size;
    struct stat info;
    rom_t *rom = (rom_t *) dev;
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL || fstat(fileno(fp), &info) < 0)
        panic("%s: bad filename %s\n", __func__, filename);

    if (base < rom->mem_size)
        panic("%s: bad base %x\n", __func__, base);

    size = ROUND_UP((base + info.st_size), 8);

    ptr = calloc(1, size);
    if (ptr == NULL)
        panic("%s: alloc memory failed!\n", __func__);

    if (rom->mem_size > 0) {
        memcpy(ptr, rom->mem_ptr, rom->mem_size);
        free(rom->mem_ptr);
        rom->mem_ptr = NULL;
    }

    if (fread(ptr + base, 1, info.st_size, fp) != info.st_size)
        panic("%s: read file failed!\n", __func__);

    rom->mem_size = size;
    rom->mem_ptr = ptr;

    fclose(fp);

    fprintf(stderr, "%s: add file %s\n", __func__, filename);

    /* Recode file-size into (base - 8) */
    if (base) {
        *((uint64_t *)(ptr + base - 8)) = info.st_size;
    }
}
