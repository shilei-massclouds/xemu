/*
 * Decode
 */

#include <stdio.h>

#include "decode.h"

uint64_t
decode(uint64_t  pc,
       uint32_t  inst,
       op_t      *op,
       uint32_t  *rd,
       uint32_t  *rs1,
       uint32_t  *rs2,
       uint64_t  *imm,
       uint32_t  *csr_addr,
       uint32_t  *opcode)
{
    if ((inst & 0x3) == 0x3) {
        /* 32-bit instruction */
        dec32(pc, inst, op, rd, rs1, rs2, imm, csr_addr, opcode);
        return pc + 4;
    } else {
        /* 16-bit instruction */
        dec16(pc, inst & 0xFFFF, op, rd, rs1, rs2, imm, csr_addr, opcode);
        return pc + 2;
    }

    return 0;
}
