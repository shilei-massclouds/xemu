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

int
main()
{
    device_t *rom;
    address_space root_as;
    uint32_t inst;
    uint64_t pc = 0x1000;

    printf("XEMU startup ...\n");

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/head.bin");

    while (1) {
        op_t      op;
        uint32_t  rd;
        uint32_t  rs1;
        uint32_t  rs2;
        uint64_t  imm;
        uint32_t  csr_addr;

        inst = read32(&root_as, pc);
        printf("[0x%lx]: \n", pc);

        pc += decode(inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

        printf("op: %s; rd: %s; rs1: %s; rs2: %s; imm: %lx; csr: %x\n\n",
               op_name(op), reg_name(rd), reg_name(rs1), reg_name(rs2),
               imm, csr_addr);
    }

    return 0;
}
