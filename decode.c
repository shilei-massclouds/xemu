/*
 * Decode
 */

#include <stdio.h>

#include "decode.h"

uint64_t
decode(uint32_t  inst,
       op_t      *op,
       uint32_t  *rd,
       uint32_t  *rs1,
       uint32_t  *rs2,
       uint64_t  *imm,
       uint32_t  *csr_addr)
{
    uint64_t  pc_inc;

    if ((inst & 0x3) == 0x3) {
        /* 32-bit instruction */
        //fprintf(stderr, "inst: %0x\n", inst);
        dec32(inst, op, rd, rs1, rs2, imm, csr_addr);
        pc_inc = 4;
    } else {
        /* 16-bit instruction */
        //fprintf(stderr, "inst: %0x\n", inst & 0xFFFF);
        dec16(inst & 0xFFFF, op, rd, rs1, rs2, imm, csr_addr);
        pc_inc = 2;
    }

    return pc_inc;
}
