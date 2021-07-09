/*
 * xemu entry
 */

#include <stdio.h>

#include "address_space.h"
#include "isa.h"
#include "operation.h"
#include "decode.h"
#include "util.h"
#include "device.h"
#include "execute.h"
#include "csr.h"
#include "regfile.h"

/*
static void
trace_decode(op_t       op,
             uint32_t   rd,
             uint32_t   rs1,
             uint32_t   rs2,
             uint64_t   imm,
             uint32_t   csr_addr)
{
    fprintf(stderr, "op: %s; rd: %s; rs1: %s; rs2: %s; imm: %lx\n",
           op_name(op),
           reg_name(rd), reg_name(rs1), reg_name(rs2), imm);

    if (op >= CSRRW && op <= CSRRCI)
        fprintf(stderr, "csr: %s\n\n", csr_name(csr_addr));
    else
        fprintf(stderr, "\n");
}
*/

static void
trace_execute(op_t       op,
              uint32_t   rd,
              uint32_t   rs1,
              uint32_t   rs2,
              uint64_t   imm,
              uint32_t   csr_addr)
{
    fprintf(stderr, "op: %s; rd: %s(0x%0lx); rs1: %s(0x%0lx); rs2: %s(0x%0lx); imm: 0x%0lx\n",
            op_name(op),
            reg_name(rd), reg[rd],
            reg_name(rs1), reg[rs1],
            reg_name(rs2), reg[rs2],
            imm);

    if (op >= CSRRW && op <= CSRRCI)
        fprintf(stderr, "csr: %s\n\n", csr_name(csr_addr));
    else
        fprintf(stderr, "\n");
}

int
main()
{
    device_t *rom;
    address_space root_as;
    uint32_t inst;
    uint64_t pc = 0x1000;

    fprintf(stderr, "XEMU startup ...\n");

    /* Init root address space */
    init_address_space(&root_as,
                       ROOT_ADDRESS_SPACE_START,
                       ROOT_ADDRESS_SPACE_END);

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/head.bin");

    ram_init(&root_as);

    uart_init(&root_as);

    while (1) {
        uint64_t next_pc;
        uint64_t new_pc;

        op_t      op;
        uint32_t  rd;
        uint32_t  rs1;
        uint32_t  rs2;
        uint64_t  imm;
        uint32_t  csr_addr;

        inst = read32(&root_as, pc, 0);
        fprintf(stderr, "[0x%lx]: \n", pc);

        next_pc = pc + decode(inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

        //trace_decode(op, rd, rs1, rs2, imm, csr_addr);

        new_pc = execute(&root_as, pc, next_pc,
                         op, rd, rs1, rs2, imm, csr_addr);

        trace_execute(op, rd, rs1, rs2, imm, csr_addr);

        pc = new_pc ? new_pc : next_pc;
    }

    return 0;
}
