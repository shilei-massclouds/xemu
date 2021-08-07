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

#define FLASH_ADDRESS_SPACE_START 0x0000000020000000
#define FLASH_ADDRESS_SPACE_END   0x0000000021FFFFFF
#define FLASH_ADDRESS_SPACE_SIZE  \
    ((size_t)FLASH_ADDRESS_SPACE_END - (size_t)FLASH_ADDRESS_SPACE_START + 1)

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
    if (addr + size > flash->mem_size) {
        panic("%s: 0x%llx out of limit 0x%lx\n",
              __func__, addr, flash->mem_size);
    }

    return (flash->mem_ptr + addr);
}

static uint64_t
flash_read(void *dev, uint64_t addr, size_t size, params_t params)
{
    uint8_t *ptr = _flash_ptr(dev, addr, size);

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

    flash->dev.as.device = flash;

    register_address_space(parent_as, &(flash->dev.as));

    return (device_t *) flash;
}

void
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

    printf("%s: add file %s\n", __func__, filename);

    /* Recode file-size into (base - 8) */
    if (base) {
        *((uint64_t *)(ptr + base - 8)) = (uint64_t)info.st_size;
    }
}
