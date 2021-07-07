/*
 * xemu entry
 */

#include <stdio.h>

#include "address_space.h"
#include "device.h"
#include "isa.h"
#include "operation.h"
#include "decode.h"
#include "util.h"

void
decode(uint32_t inst)
{
      op_t      op;
      uint32_t  rd;
      uint32_t  rs1;
      uint32_t  rs2;
      uint64_t  imm;
      uint32_t  csr_addr;

      dec32(inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

      printf("op: %x; rd: %s; rs1: %s; rs2: %s; imm: %lx; csr: %x\n",
             op, reg_name(rd), reg_name(rs1), reg_name(rs2),
             imm, csr_addr);
}

int
main()
{
    device_t *rom;
    address_space root_as;
    uint32_t inst;

    printf("XEMU startup ...\n");

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/head.bin");

    inst = read32(&root_as, 0x1000);
    printf("[0x1000]: %x\n", inst);

    if ((inst & 0x3) == 0x3)
        decode(inst);
    else
        printf("This is a compressed instruction!\n");

    return 0;
}
