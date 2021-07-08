/*
 * Execute
 */

#include "execute.h"

#include "regfile.h"
#include "csr.h"
#include "util.h"

void
execute(op_t     op,
        uint32_t rd,
        uint32_t rs1,
        uint32_t rs2,
        uint64_t imm,
        uint32_t csr_addr)
{
    int with_imm = 0;

    switch (op)
    {
    case NOP:
        break;

    case LUI:
        reg[rd] = imm;
        break;

    case CSRRWI:
        with_imm = 1;
    case CSRRW:
        csr[csr_addr] = with_imm ? imm : reg[rs1];
        break;

    case CSRRSI:
        with_imm = 1;
    case CSRRS:
        csr[csr_addr] = csr[csr_addr] | (with_imm ? imm : reg[rs1]);
        break;

    case CSRRCI:
        with_imm = 1;
    case CSRRC:
        csr[csr_addr] = csr[csr_addr] & ~(with_imm ? imm : reg[rs1]);
        break;

    default:
        panic("%s: bad op (%s)\n", __func__, op_name(op));
    }
}
