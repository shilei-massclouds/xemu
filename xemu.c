/*
 * xemu entry
 */

#include <stdio.h>

#include "address_space.h"
#include "device.h"

int
main()
{
    device_t *rom;
    address_space root_as;

    printf("XEMU startup ...\n");

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/head.bin");

    printf("read [0x1000]: %lx\n", read64(&root_as, 0x1000));

    return 0;
}
