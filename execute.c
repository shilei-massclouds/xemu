/*
 * Execute
 */

#include "types.h"
#include "execute.h"

#include "regfile.h"
#include "csr.h"
#include "util.h"
#include "trap.h"

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
    uint64_t rd_val;
    double   frd_val;
    bool     is_frd = false;
    uint64_t ret_pc = 0;

    bool has_except = false;

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
        ret_pc = pc + imm;
        break;

    case JALR:
        rd_val = next_pc;
        ret_pc = reg[rs1] + imm;
        break;

    case BEQ:
        if (reg[rs1] == reg[rs2])
            ret_pc = pc + imm;
        break;

    case BNE:
        if (reg[rs1] != reg[rs2])
            ret_pc = pc + imm;
        break;

    case BLT:
        if ((int64_t)reg[rs1] < (int64_t)reg[rs2])
            ret_pc = pc + imm;
        break;

    case BGE:
        if ((int64_t)reg[rs1] >= (int64_t)reg[rs2])
            ret_pc = pc + imm;
        break;

    case BLTU:
        if (reg[rs1] < reg[rs2])
            ret_pc = pc + imm;
        break;

    case BGEU:
        if (reg[rs1] >= reg[rs2])
            ret_pc = pc + imm;
        break;

    case LB:
        addr = reg[rs1] + imm;
        rd_val = (int8_t)read(as, addr, 1, 0, &has_except);
        break;

    case LH:
        addr = reg[rs1] + imm;
        rd_val = (int16_t)read(as, addr, 2, 0, &has_except);
        break;

    case LW:
        addr = reg[rs1] + imm;
        rd_val = (int32_t)read(as, addr, 4, 0, &has_except);
        break;

    case LD:
        addr = reg[rs1] + imm;
        rd_val = read(as, addr, 8, 0, &has_except);
        break;

    case LBU:
        addr = reg[rs1] + imm;
        rd_val = (uint8_t)read(as, addr, 1, 0, &has_except);
        break;

    case LHU:
        addr = reg[rs1] + imm;
        rd_val = (uint16_t)read(as, addr, 2, 0, &has_except);
        break;

    case LWU:
        addr = reg[rs1] + imm;
        rd_val = (uint32_t)read(as, addr, 4, 0, &has_except);
        break;

    case SB:
        addr = reg[rs1] + imm;
        write(as, addr, 1, reg[rs2], 0, &has_except);
        break;

    case SH:
        addr = reg[rs1] + imm;
        write(as, addr, 2, reg[rs2], 0, &has_except);
        break;

    case SW:
        addr = reg[rs1] + imm;
        write(as, addr, 4, reg[rs2], 0, &has_except);
        break;

    case SD:
        addr = reg[rs1] + imm;
        write(as, addr, 8, reg[rs2], 0, &has_except);
        break;

    case ADDI:
        rd_val = reg[rs1] + imm;
        break;

    case ADDIW:
        rd_val = TO_WORD((int32_t)reg[rs1] + imm);
        break;

    case ADD:
        rd_val = reg[rs1] + reg[rs2];
        break;

    case SLT:
        rd_val = ((int64_t)reg[rs1] < (int64_t)reg[rs2]) ? 1 : 0;
        break;

    case SLTU:
        rd_val = (reg[rs1] < reg[rs2]) ? 1 : 0;
        break;

    case ADDW:
        rd_val = TO_WORD((int32_t)reg[rs1] + (int32_t)reg[rs2]);
        break;

    case SLTI:
        rd_val = ((int64_t)reg[rs1] < (int64_t)imm) ? 1 : 0;
        break;

    case SLTIU:
        rd_val = (reg[rs1] < imm) ? 1 : 0;
        break;

    case SUB:
        rd_val = reg[rs1] - reg[rs2];
        break;

    case SUBW:
        rd_val = TO_WORD((int32_t)reg[rs1] - (int32_t)reg[rs2]);
        break;

    case SLLI:
        rd_val = reg[rs1] << BITS(imm, 5, 0);
        break;

    case SLLIW:
        rd_val = TO_WORD((uint32_t)reg[rs1] << BITS(imm, 4, 0));
        break;

    case SLL:
        rd_val = reg[rs1] << BITS(reg[rs2], 5, 0);
        break;

    case SLLW:
        rd_val = TO_WORD((uint32_t)reg[rs1] << BITS(reg[rs2], 4, 0));
        break;

    case XORI:
        rd_val = reg[rs1] ^ imm;
        break;

    case XOR:
        rd_val = reg[rs1] ^ reg[rs2];
        break;

    case SRLI:
        rd_val = reg[rs1] >> BITS(imm, 5, 0);
        break;

    case SRLIW:
        rd_val = TO_WORD((uint32_t)reg[rs1] >> BITS(imm, 4, 0));
        break;

    case SRL:
        rd_val = reg[rs1] >> BITS(reg[rs2], 5, 0);
        break;

    case SRLW:
        rd_val = TO_WORD((uint32_t)reg[rs1] >> BITS(reg[rs2], 4, 0));
        break;

    case SRAI:
        rd_val = ((int64_t)reg[rs1]) >> BITS(imm, 5, 0);
        break;

    case SRAIW:
        rd_val = (int64_t)(((int32_t)reg[rs1]) >> BITS(imm, 5, 0));
        break;

    case SRA:
        rd_val = ((int64_t)reg[rs1]) >> BITS(reg[rs2], 5, 0);
        break;

    case SRAW:
        rd_val = (int64_t)(((int32_t)reg[rs1]) >> BITS(reg[rs2], 5, 0));
        break;

    case ORI:
        rd_val = reg[rs1] | imm;
        break;

    case OR:
        rd_val = reg[rs1] | reg[rs2];
        break;

    case ANDI:
        rd_val = reg[rs1] & imm;
        break;

    case AND:
        rd_val = reg[rs1] & reg[rs2];
        break;

    case FENCE:
        break;

    case FENCE_I:
        break;

    case ECALL:
        ret_pc = trap_enter(pc, (CAUSE_ECALL_FROM_U_MODE + priv), 0);
        break;

    case URET:
    case SRET:
    case MRET:
        ret_pc = trap_exit(op);
        break;

    case WFI:
        break;

    case SFENCE_VMA:
        break;

    case CSRRW:
        rd_val = csr_update(csr_addr, reg[rs1], CSR_OP_WRITE, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);

        /*
        if (csr_addr == 0)
            fprintf(stderr, "#DEBUG:[%lx]: %lx\n", pc, reg[rs1]);
        */
        break;

    case CSRRS:
        rd_val = csr_update(csr_addr, reg[rs1], CSR_OP_SET, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);
        break;

    case CSRRC:
        rd_val = csr_update(csr_addr, reg[rs1], CSR_OP_CLEAR, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);
        break;

    case CSRRWI:
        rd_val = csr_update(csr_addr, imm, CSR_OP_WRITE, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);
        break;

    case CSRRSI:
        rd_val = csr_update(csr_addr, imm, CSR_OP_SET, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);
        break;

    case CSRRCI:
        rd_val = csr_update(csr_addr, imm, CSR_OP_CLEAR, &has_except);
        if (has_except)
            ret_pc = trap_enter(pc, CAUSE_ILLEGAL_INST, 0);
        break;

    case MUL:
        rd_val = (int64_t)reg[rs1] * (int64_t)reg[rs2];
        break;

    case MULH:
        rd_val = ((__int128_t)reg[rs1] * (__int128_t)reg[rs2]) >> 64;
        break;

    case MULHSU:
        rd_val = ((__int128_t)reg[rs1] * reg[rs2]) >> 64;
        break;

    case MULHU:
        rd_val = ((__uint128_t)reg[rs1] *(__uint128_t) reg[rs2]) >> 64;
        break;

    case DIV:
        if (reg[rs2] == 0)
            rd_val = -1;
        else if (((int64_t)reg[rs2] == -1) && (reg[rs1] == (1UL << 63)))
            rd_val = (1UL << 63);
        else
            rd_val = (int64_t)reg[rs1] / (int64_t)reg[rs2];

        break;

    case DIVU:
        if (reg[rs2] == 0)
            rd_val = (uint64_t) -1;
        else
            rd_val = reg[rs1] / reg[rs2];

        break;

    case REM:
        if (reg[rs2] == 0)
            rd_val = (int64_t)reg[rs1];
        else if (((int64_t)reg[rs2] == -1) && (reg[rs1] == (1UL << 63)))
            rd_val = 0;
        else
            rd_val = (int64_t)reg[rs1] % (int64_t)reg[rs2];

        break;

    case REMU:
        if (reg[rs2] == 0)
            rd_val = reg[rs1];
        else
            rd_val = reg[rs1] % reg[rs2];

        break;

    case MULW:
        rd_val = TO_WORD((int32_t)reg[rs1] * (int32_t)reg[rs2]);
        break;

    case DIVW:
        if ((int32_t)reg[rs2] == 0)
            rd_val = (int32_t) -1;
        else if (((int32_t)reg[rs2] == -1) && ((uint32_t)reg[rs1] == (1 << 31)))
            rd_val = (int32_t)(1 << 31);
        else
            rd_val = TO_WORD((int32_t)reg[rs1] / (int32_t)reg[rs2]);

        break;

    case DIVUW:
        if ((int32_t)reg[rs2] == 0)
            rd_val = (uint32_t) -1;
        else
            rd_val = TO_WORD((uint32_t)reg[rs1] / (uint32_t)reg[rs2]);

        break;

    case REMW:
        if ((int32_t)reg[rs2] == 0)
            rd_val = (int32_t)reg[rs1];
        else if (((int32_t)reg[rs2] == -1) && ((uint32_t)reg[rs1] == (1 << 31)))
            rd_val = 0;
        else
            rd_val = TO_WORD((int32_t)reg[rs1] % (int32_t)reg[rs2]);

        break;

    case REMUW:
        if ((uint32_t)reg[rs2] == 0)
            rd_val = (uint32_t)reg[rs1];
        else
            rd_val = TO_WORD((uint32_t)reg[rs1] % (uint32_t)reg[rs2]);

        break;

    case LR_D:
        rd_val = read(as, reg[rs1], 8, PARAMS_LR_SC, &has_except);
        break;
    case SC_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_LR_SC, &has_except);
        break;
    case AMO_ADD_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_ADD, &has_except);
        break;
    case AMO_SWAP_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_SWAP, &has_except);
        break;
    case AMO_XOR_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_XOR, &has_except);
        break;
    case AMO_OR_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_OR, &has_except);
        break;
    case AMO_AND_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_AND, &has_except);
        break;
    case AMO_MIN_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_MIN, &has_except);
        break;
    case AMO_MAX_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_MAX, &has_except);
        break;
    case AMO_MINU_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_MINU, &has_except);
        break;
    case AMO_MAXU_D:
        rd_val = write(as, reg[rs1], 8, reg[rs2], PARAMS_AMO_MAXU, &has_except);
        break;

    case LR_W:
        rd_val = (int32_t)read(as, reg[rs1], 4, PARAMS_LR_SC, &has_except);
        break;
    case SC_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_LR_SC, &has_except);
        break;
    case AMO_ADD_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_ADD, &has_except);
        break;
    case AMO_SWAP_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_SWAP, &has_except);
        break;
    case AMO_XOR_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_XOR, &has_except);
        break;
    case AMO_OR_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_OR, &has_except);
        break;
    case AMO_AND_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_AND, &has_except);
        break;
    case AMO_MIN_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_MIN, &has_except);
        break;
    case AMO_MAX_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_MAX, &has_except);
        break;
    case AMO_MINU_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_MINU, &has_except);
        break;
    case AMO_MAXU_W:
        rd_val = write(as, reg[rs1], 4, reg[rs2], PARAMS_AMO_MAXU, &has_except);
        break;

    /* Floating-point instructions */

    case FLW:
        addr = reg[rs1] + imm;
        frd_val = (float)read(as, addr, 4, 0, &has_except);
        is_frd = true;
        break;

    case FMV_W_X:
        frd_val = (float) reg[rs1];
        is_frd = true;
        break;

    case FLD:
        addr = reg[rs1] + imm;
        frd_val = (double)read(as, addr, 8, 0, &has_except);
        is_frd = true;
        break;

    default:
        panic("%s: bad op (%s) at: %x\n", __func__, op_name(op), pc);
    }

    if (is_frd)
        freg[rd] = frd_val;
    else if (rd)
        reg[rd] = rd_val;

    return ret_pc;
}
