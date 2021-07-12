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
#include "mmu.h"
#include "trap.h"

static int trace_decode_en;
static int trace_execute_en = 1;
static uint64_t trace_pc_start = 0xffffffe00001b9a6;

static void
trace_decode(uint64_t   pc,
             op_t       op,
             uint32_t   rd,
             uint32_t   rs1,
             uint32_t   rs2,
             uint64_t   imm,
             uint32_t   csr_addr)
{
    if (!trace_decode_en || pc < trace_pc_start)
        return;

    fprintf(stderr, "[%lx]: %s; rd: %s; rs1: %s; rs2: %s; imm: %lx\n",
           pc, op_name(op),
           reg_name(rd), reg_name(rs1), reg_name(rs2), imm);

    if (op >= CSRRW && op <= CSRRCI)
        fprintf(stderr, "csr: %s\n\n", csr_name(csr_addr));
    else
        fprintf(stderr, "\n");
}

static void
trace_execute(uint64_t   pc,
              op_t       op,
              uint32_t   rd,
              uint32_t   rs1,
              uint32_t   rs2,
              uint64_t   imm,
              uint32_t   csr_addr)
{
    if (!trace_execute_en || pc < trace_pc_start)
        return;

    fprintf(stderr, "[%lx]: %s; rd: %s(0x%0lx); rs1: %s(0x%0lx); rs2: %s(0x%0lx); imm: 0x%0lx\n",
            pc,
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

uint32_t
fetch(address_space *as, uint64_t pc, int *except)
{
    uint32_t lo;
    uint32_t hi;

    if ((pc + 2) & (PAGE_SIZE - 1UL))
        return read(as, pc, 4, 0, except);

    lo = read(as, pc, 2, 0, except);
    if (*except)
        return 0;

    hi = read(as, pc + 2, 2, 0, except);
    if (*except)
        return 0;

    return ((hi << 16) | lo);
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

    /* Init CSR */
    csr_init();

    rom = rom_init(&root_as);
    rom_add_file(rom, "image/head.bin", 0);
    rom_add_file(rom, "image/virt.dtb", 0x100);
    rom_add_file(rom, "image/fw_jump.bin", 0x2000);
    rom_add_file(rom, "image/payload.bin", 0x20000);

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
        int except = 0;

        /* Fetch */
        inst = fetch(&root_as, pc, &except);

        if (except) {
            pc = trap_enter(pc, CAUSE_INST_PAGE_FAULT, pc);
            continue;
        }

        /* Decode */
        next_pc = pc + decode(inst, &op, &rd, &rs1, &rs2, &imm, &csr_addr);

        trace_decode(pc, op, rd, rs1, rs2, imm, csr_addr);

        /* Execute */
        new_pc = execute(&root_as, pc, next_pc,
                         op, rd, rs1, rs2, imm, csr_addr);

        trace_execute(pc, op, rd, rs1, rs2, imm, csr_addr);

        pc = new_pc ? new_pc : next_pc;
    }

    return 0;
}
