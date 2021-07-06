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
#define ROM_ADDRESS_SPACE_END   0x000000000FFFF000

typedef struct _rom
{
    device_t dev;

    uint8_t *mem_ptr;
    size_t mem_size;
} rom_t;

static uint64_t
rom_read64(void *dev, uint64_t addr)
{
    rom_t *rom = (rom_t *) dev;
    if (addr + 8 >= rom->mem_size) {
        panic("%s: 0x%llx out of limit 0x%lx\n",
              __func__, addr, rom->mem_size);
    }

    return *((uint64_t *)(rom->mem_ptr + addr));
}

static void
rom_write64(void *dev, uint64_t addr, uint64_t data)
{
    rom_t *rom = (rom_t *) dev;
    if (addr + 8 >= rom->mem_size) {
        panic("%s: 0xllx out of limit 0x%lx\n",
              __func__, addr, rom->mem_size);
    }

    *((uint64_t *)(rom->mem_ptr + addr)) = data;
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

    rom->dev.as.ops.read64_op = &rom_read64;
    rom->dev.as.ops.write64_op = &rom_write64;

    rom->dev.as.device = rom;

    register_address_space(parent_as, &(rom->dev.as));

    return (device_t *) rom;
}

void
rom_add_file(device_t *dev, const char *filename)
{
    uint8_t *ptr;
    struct stat info;
    rom_t *rom = (rom_t *) dev;
    FILE *fp = fopen(filename, "rb");

    if (fp == NULL || fstat(fileno(fp), &info) < 0)
        panic("%s: bad filename %s\n", __func__, filename);

    ptr = malloc(rom->mem_size + info.st_size);
    if (ptr == NULL)
        panic("%s: alloc memory failed!\n", __func__);

    if (rom->mem_size > 0) {
        memcpy(ptr, rom->mem_ptr, rom->mem_size);
        free(rom->mem_ptr);
        rom->mem_ptr = NULL;
    }

    if (fread(ptr + rom->mem_size, 1, info.st_size, fp) != info.st_size)
        panic("%s: read file failed!\n", __func__);

    rom->mem_size += info.st_size;
    rom->mem_ptr = ptr;

    fclose(fp);

    printf("%s: add file %s\n", __func__, filename);
}
