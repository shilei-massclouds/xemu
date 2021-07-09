/*
 * Execute
 */

#include "execute.h"

#include "regfile.h"
#include "csr.h"
#include "util.h"

uint64_t
execute(address_space *as,
        uint64_t pc,
        uint64_t next_pc,
        op_t     op,
        uint32_t rd,
        uint32_t rs1,
        uint32_t rs2,
        uint64_t imm,
        uint32_t csr_addr)
{
    uint64_t addr;
    uint64_t new_pc = 0;

    switch (op)
    {
    case NOP:
        break;

    case LUI:
        reg[rd] = imm;
        break;

    case JAL:
        reg[rd] = next_pc;
        new_pc = pc + imm;
        break;

    case BEQ:
        if (reg[rs1] == reg[rs2])
            new_pc = pc + imm;
        break;

    case BNE:
        if (reg[rs1] != reg[rs2])
            new_pc = pc + imm;
        break;

    case BLT:
        if ((int64_t)reg[rs1] < (int64_t)reg[rs2])
            new_pc = pc + imm;
        break;

    case BGE:
        if ((int64_t)reg[rs1] >= (int64_t)reg[rs2])
            new_pc = pc + imm;
        break;

    case BLTU:
        if (reg[rs1] < reg[rs2])
            new_pc = pc + imm;
        break;

    case BGEU:
        if (reg[rs1] >= reg[rs2])
            new_pc = pc + imm;
        break;

    case LB:
        addr = reg[rs1] + imm;
        reg[rd] = (int8_t)read8(as, addr, 0);
        break;

    case LH:
        addr = reg[rs1] + imm;
        reg[rd] = (int16_t)read16(as, addr, 0);
        break;

    case LW:
        addr = reg[rs1] + imm;
        reg[rd] = (int32_t)read32(as, addr, 0);
        break;

    case LD:
        addr = reg[rs1] + imm;
        reg[rd] = read64(as, addr, 0);
        break;

    case LBU:
        addr = reg[rs1] + imm;
        reg[rd] = read8(as, addr, 0);
        break;

    case LHU:
        addr = reg[rs1] + imm;
        reg[rd] = read16(as, addr, 0);
        break;

    case LWU:
        addr = reg[rs1] + imm;
        reg[rd] = read32(as, addr, 0);
        break;

    case SB:
        addr = reg[rs1] + imm;
        write8(as, addr, reg[rs2], 0);
        break;

    case SH:
        addr = reg[rs1] + imm;
        write16(as, addr, reg[rs2], 0);
        break;

    case SW:
        addr = reg[rs1] + imm;
        write32(as, addr, reg[rs2], 0);
        break;

    case SD:
        addr = reg[rs1] + imm;
        write64(as, addr, reg[rs2], 0);
        break;

    case ADDI:
        reg[rd] = reg[rs1] + imm;
        break;

    case ADDIW:
        reg[rd] = TO_WORD(reg[rs1] + imm);
        break;

    case ADD:
        reg[rd] = reg[rs1] + reg[rs2];
        break;

    case ADDW:
        reg[rd] = TO_WORD(reg[rs1] + reg[rs2]);
        break;

    case SLLI:
        reg[rd] = reg[rs1] << BITS(imm, 5, 0);
        break;

    case SLLIW:
        reg[rd] = TO_WORD(reg[rs1] << BITS(imm, 4, 0));
        break;

    case SRLI:
        reg[rd] = reg[rs1] >> BITS(imm, 5, 0);
        break;

    case SRLIW:
        reg[rd] = TO_WORD(reg[rs1] >> BITS(imm, 4, 0));
        break;

    case SRAI:
        reg[rd] = ((int64_t)reg[rs1]) >> BITS(imm, 5, 0);
        break;

    case SRAIW:
        reg[rd] = TO_WORD(((int64_t)reg[rs1]) >> BITS(imm, 5, 0));
        break;

    case WFI:
        break;

    case CSRRW:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = reg[rs1];
        break;

    case CSRRS:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] | reg[rs1];
        break;

    case CSRRC:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] & ~reg[rs1];
        break;

    case CSRRWI:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = imm;
        break;

    case CSRRSI:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] | imm;
        break;

    case CSRRCI:
        reg[rd] = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] & ~imm;
        break;

    case LR_D:
        reg[rd] = read64(as, reg[rs1], PARAMS_LR_SC);
        break;
    case SC_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_LR_SC);
        break;
    case AMO_ADD_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_ADD);
        break;
    case AMO_SWAP_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_SWAP);
        break;
    case AMO_XOR_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_XOR);
        break;
    case AMO_OR_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_OR);
        break;
    case AMO_AND_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_AND);
        break;
    case AMO_MIN_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MIN);
        break;
    case AMO_MAX_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MAX);
        break;
    case AMO_MINU_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MINU);
        break;
    case AMO_MAXU_D:
        reg[rd] = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MAXU);
        break;

    case LR_W:
        reg[rd] = (int32_t)read32(as, reg[rs1], PARAMS_LR_SC);
        break;
    case SC_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_LR_SC);
        break;
    case AMO_ADD_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_ADD);
        break;
    case AMO_SWAP_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_SWAP);
        break;
    case AMO_XOR_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_XOR);
        break;
    case AMO_OR_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_OR);
        break;
    case AMO_AND_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_AND);
        break;
    case AMO_MIN_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MIN);
        break;
    case AMO_MAX_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MAX);
        break;
    case AMO_MINU_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MINU);
        break;
    case AMO_MAXU_W:
        reg[rd] = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MAXU);
        break;

    default:
        panic("%s: bad op (%s)\n", __func__, op_name(op));
    }

    return new_pc;
}
