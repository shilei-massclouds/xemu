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
    uint64_t rd_val;

    switch (op)
    {
    case NOP:
        break;

    case LUI:
        rd_val = imm;
        break;

    case AUIPC:
        rd_val = pc + imm;
        break;

    case JAL:
        rd_val = next_pc;
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
        rd_val = (int8_t)read8(as, addr, 0);
        break;

    case LH:
        addr = reg[rs1] + imm;
        rd_val = (int16_t)read16(as, addr, 0);
        break;

    case LW:
        addr = reg[rs1] + imm;
        rd_val = (int32_t)read32(as, addr, 0);
        break;

    case LD:
        addr = reg[rs1] + imm;
        rd_val = read64(as, addr, 0);
        break;

    case LBU:
        addr = reg[rs1] + imm;
        rd_val = read8(as, addr, 0);
        break;

    case LHU:
        addr = reg[rs1] + imm;
        rd_val = read16(as, addr, 0);
        break;

    case LWU:
        addr = reg[rs1] + imm;
        rd_val = read32(as, addr, 0);
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
        rd_val = reg[rs1] + imm;
        break;

    case ADDIW:
        rd_val = TO_WORD(reg[rs1] + imm);
        break;

    case ADD:
        rd_val = reg[rs1] + reg[rs2];
        break;

    case ADDW:
        rd_val = TO_WORD(reg[rs1] + reg[rs2]);
        break;

    case SLLI:
        rd_val = reg[rs1] << BITS(imm, 5, 0);
        break;

    case SLLIW:
        rd_val = TO_WORD(reg[rs1] << BITS(imm, 4, 0));
        break;

    case SRLI:
        rd_val = reg[rs1] >> BITS(imm, 5, 0);
        break;

    case SRLIW:
        rd_val = TO_WORD(reg[rs1] >> BITS(imm, 4, 0));
        break;

    case SRAI:
        rd_val = ((int64_t)reg[rs1]) >> BITS(imm, 5, 0);
        break;

    case SRAIW:
        rd_val = TO_WORD(((int64_t)reg[rs1]) >> BITS(imm, 5, 0));
        break;

    case FENCE:
        break;

    case FENCE_I:
        break;

    case WFI:
        break;

    case CSRRW:
        rd_val = csr[csr_addr];
        csr[csr_addr] = reg[rs1];
        break;

    case CSRRS:
        rd_val = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] | reg[rs1];
        break;

    case CSRRC:
        rd_val = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] & ~reg[rs1];
        break;

    case CSRRWI:
        rd_val = csr[csr_addr];
        csr[csr_addr] = imm;
        break;

    case CSRRSI:
        rd_val = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] | imm;
        break;

    case CSRRCI:
        rd_val = csr[csr_addr];
        csr[csr_addr] = csr[csr_addr] & ~imm;
        break;

    case LR_D:
        rd_val = read64(as, reg[rs1], PARAMS_LR_SC);
        break;
    case SC_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_LR_SC);
        break;
    case AMO_ADD_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_ADD);
        break;
    case AMO_SWAP_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_SWAP);
        break;
    case AMO_XOR_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_XOR);
        break;
    case AMO_OR_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_OR);
        break;
    case AMO_AND_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_AND);
        break;
    case AMO_MIN_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MIN);
        break;
    case AMO_MAX_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MAX);
        break;
    case AMO_MINU_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MINU);
        break;
    case AMO_MAXU_D:
        rd_val = write64(as, reg[rs1], reg[rs2], PARAMS_AMO_MAXU);
        break;

    case LR_W:
        rd_val = (int32_t)read32(as, reg[rs1], PARAMS_LR_SC);
        break;
    case SC_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_LR_SC);
        break;
    case AMO_ADD_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_ADD);
        break;
    case AMO_SWAP_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_SWAP);
        break;
    case AMO_XOR_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_XOR);
        break;
    case AMO_OR_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_OR);
        break;
    case AMO_AND_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_AND);
        break;
    case AMO_MIN_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MIN);
        break;
    case AMO_MAX_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MAX);
        break;
    case AMO_MINU_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MINU);
        break;
    case AMO_MAXU_W:
        rd_val = write32(as, reg[rs1], reg[rs2], PARAMS_AMO_MAXU);
        break;

    default:
        panic("%s: bad op (%s)\n", __func__, op_name(op));
    }

    if (rd)
        reg[rd] = rd_val;

    return new_pc;
}
