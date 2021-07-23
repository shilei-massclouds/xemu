/*
 * Decode 32-bit instruction
 */

#include <stdint.h>

#include "isa.h"
#include "operation.h"
#include "util.h"

#define I_IMM(INST) ((EXPAND_BIT(INST, 31, 52) << 12) | BITS(INST, 31, 20))

#define S_IMM(INST) ((EXPAND_BIT(INST, 31, 52) << 12) | \
                     (BITS(INST, 31, 25) << 5) | \
                     BITS(INST, 11, 7))

#define B_IMM(INST) ((EXPAND_BIT(INST, 31, 52) << 12) | \
                     (BIT(INST, 7) << 11) | \
                     (BITS(INST, 30, 25) << 5) | \
                     (BITS(INST, 11, 8) << 1))

#define U_IMM(INST) ((EXPAND_BIT(INST, 31, 32) << 32) | \
                     (BITS(INST, 31, 12) << 12))

#define J_IMM(INST) ((EXPAND_BIT(INST, 31, 44) << 20) | \
                     (BITS(INST, 19, 12) << 12) |\
                     (BIT(INST, 20) << 11) | \
                     (BITS(INST, 30, 21) << 1))

#define C_IMM(INST) BITS(INST, 19, 15)

#define M_IMM6(INST) BITS(INST, 25, 20)
#define M_IMM5(INST) BITS(INST, 24, 20)


void
dec32(uint64_t  pc,
      uint32_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr)
{
    uint32_t opcode = BITS(inst, 6, 0);
    uint32_t funct3 = BITS(inst, 14, 12);
    uint32_t funct5 = BITS(inst, 31, 27);
    uint32_t funct7 = BITS(inst, 31, 25);

    *op = NOP;
    *rd = BITS(inst, 11, 7);
    *rs1 = BITS(inst, 19, 15);
    *rs2 = BITS(inst, 24, 20);
    *imm = 0;
    *csr_addr = 0;

    switch (opcode)
    {
    case OP_LUI:
        *op = LUI;
        *imm = U_IMM(inst);
        break;

    case OP_AUIPC:
        *op = AUIPC;
        *imm = U_IMM(inst);
        break;

    case OP_JAL:
        *op = JAL;
        *imm = J_IMM(inst);
        break;

    case OP_JALR:
        *op = JALR;
        *imm = I_IMM(inst);
        break;

    case OP_BRANCH:
        switch (funct3)
        {
        case 0:
            *op = BEQ;
            break;
        case 1:
            *op = BNE;
            break;
        case 4:
            *op = BLT;
            break;
        case 5:
            *op = BGE;
            break;
        case 6:
            *op = BLTU;
            break;
        case 7:
            *op = BGEU;
            break;
        default:
            panic("%s: bad branch instruction (0x%x)\n", __func__, inst);
        }

        *imm = B_IMM(inst);
        *rd = 0;
        break;

    case OP_LOAD:
        switch (funct3)
        {
        case 0:
            *op = LB;
            break;
        case 1:
            *op = LH;
            break;
        case 2:
            *op = LW;
            break;
        case 3:
            *op = LD;
            break;
        case 4:
            *op = LBU;
            break;
        case 5:
            *op = LHU;
            break;
        case 6:
            *op = LWU;
            break;
        default:
            panic("%s: bad load instruction (0x%x)\n", __func__, inst);
        }

        *imm = I_IMM(inst);
        break;

    case OP_STORE:
        switch (funct3)
        {
        case 0:
            *op = SB;
            break;
        case 1:
            *op = SH;
            break;
        case 2:
            *op = SW;
            break;
        case 3:
            *op = SD;
            break;
        default:
            panic("%s: bad store instruction (0x%x)\n", __func__, inst);
        }

        *imm = S_IMM(inst);
        *rd = 0;
        break;

    case OP_LOAD_FP:
        switch (funct3)
        {
        case 2:
            *op = FLW;
            break;
        case 3:
            *op = FLD;
            break;
        default:
            panic("%s: bad load-fp instruction (0x%x)\n", __func__, inst);
        }

        *imm = I_IMM(inst);
        break;

    case OP_IMM:
        switch (funct3)
        {
        case 0:
            *op = ADDI;
            break;
        case 1:
            *op = SLLI;
            break;
        case 2:
            *op = SLTI;
            break;
        case 3:
            *op = SLTIU;
            break;
        case 4:
            *op = XORI;
            break;
        case 5:
            *op = BIT(inst, 30) ? SRAI : SRLI;
            break;
        case 6:
            *op = ORI;
            break;
        case 7:
            *op = ANDI;
            break;
        default:
            panic("%s: bad imm instruction (0x%x)\n", __func__, inst);
        }

        *imm = (funct3 == 1 || funct3 == 5) ? M_IMM6(inst) : I_IMM(inst);
        break;

    case OP_IMM_W:
        switch (funct3)
        {
        case 0:
            *op = ADDIW;
            *imm = I_IMM(inst);
            break;
        case 1:
            *op = SLLIW;
            *imm = M_IMM5(inst);
            break;
        case 5:
            *op = BIT(inst, 30) ? SRAIW : SRLIW;
            *imm = M_IMM5(inst);
            break;
        default:
            panic("%s: bad imm_w instruction (0x%x)\n", __func__, inst);
        }

        break;

    case OP_REG:
        if (funct7 == 1) {
            switch (funct3)
            {
            case 0:
                *op = MUL;
                break;
            case 1:
                *op = MULH;
                break;
            case 2:
                *op = MULHSU;
                break;
            case 3:
                *op = MULHU;
                break;
            case 4:
                *op = DIV;
                break;
            case 5:
                *op = DIVU;
                break;
            case 6:
                *op = REM;
                break;
            case 7:
                *op = REMU;
                break;
            default:
                panic("%s: bad mul/div instruction (0x%x)\n", __func__, inst);
            }
        } else {
            switch (funct3)
            {
            case 0:
                *op = BIT(inst, 30) ? SUB : ADD;
                break;
            case 1:
                *op = SLL;
                break;
            case 2:
                *op = SLT;
                break;
            case 3:
                *op = SLTU;
                break;
            case 4:
                *op = XOR;
                break;
            case 5:
                *op = BIT(inst, 30) ? SRA : SRL;
                break;
            case 6:
                *op = OR;
                break;
            case 7:
                *op = AND;
                break;
            default:
                panic("%s: bad reg instruction (0x%x)\n", __func__, inst);
            }
        }

        break;

    case OP_REG_W:
        if (funct7 == 1) {
            switch (funct3)
            {
            case 0:
                *op = MULW;
                break;
            case 4:
                *op = DIVW;
                break;
            case 5:
                *op = DIVUW;
                break;
            case 6:
                *op = REMW;
                break;
            case 7:
                *op = REMUW;
                break;
            default:
                panic("%s: bad mul(w)/div(w) instruction (0x%x)\n", __func__, inst);
            }
        } else {
            switch (funct3)
            {
            case 0:
                *op = BIT(inst, 30) ? SUBW : ADDW;
                break;
            case 1:
                *op = SLLW;
                break;
            case 5:
                *op = BIT(inst, 30) ? SRAW : SRLW;
                break;
            default:
                panic("%s: bad reg_w instruction (0x%x)", __func__, inst);
            }
        }

        break;

    case OP_MISC:
        switch (funct3)
        {
        case 0:
            *op = FENCE;
            break;
        case 1:
            *op = FENCE_I;
            break;
        default:
            panic("%s: bad misc instruction (0x%x)", __func__, inst);
        }

        break;

    case OP_SYSTEM:
        switch (funct3)
        {
        case 0:
            switch (funct7)
            {
            case 0:
                switch (BITS(inst, 24, 20))
                {
                case 0:
                    *op = ECALL;
                    break;
                case 1:
                    *op = EBREAK;
                    break;
                case 2:
                    *op = URET;
                    break;
                default:
                    panic("%s: bad system instruction (0x%x)", __func__, inst);
                }
                break;
            case 8:
                if (BITS(inst, 24, 20) == 2)
                    *op = SRET;
                else if (BITS(inst, 24, 20) == 5)
                    *op = WFI;
                else
                    panic("%s: bad system instruction (0x%x)", __func__, inst);
                break;
            case 9:
                *op = SFENCE_VMA;
                break;
            case 24:
                *op = MRET;
                break;
            default:
                panic("%s: bad system instruction (0x%x)", __func__, inst);
            }

            break;
        case 1:
            *op = CSRRW;
            break;
        case 2:
            *op = CSRRS;
            break;
        case 3:
            *op = CSRRC;
            break;
        case 5:
            *op = CSRRWI;
            *imm = C_IMM(inst);
            break;
        case 6:
            *op = CSRRSI;
            *imm = C_IMM(inst);
            break;
        case 7:
            *op = CSRRCI;
            *imm = C_IMM(inst);
            break;
        default:
            panic("%s: bad system instruction (0x%x)", __func__, inst);
        }

        *csr_addr = BITS(inst, 31, 20);
        break;

    case OP_AMO:
        switch (funct5)
        {
        case 0:
            *op = (funct3 == 3) ? AMO_ADD_D : AMO_ADD_W;
            break;
        case 1:
            *op = (funct3 == 3) ? AMO_SWAP_D : AMO_SWAP_W;
            break;
        case 2:
            *op = (funct3 == 3) ? LR_D : LR_W;
            break;
        case 3:
            *op = (funct3 == 3) ? SC_D : SC_W;
            break;
        case 4:
            *op = (funct3 == 3) ? AMO_XOR_D : AMO_XOR_W;
            break;
        case 8:
            *op = (funct3 == 3) ? AMO_OR_D : AMO_OR_W;
            break;
        case 12:
            *op = (funct3 == 3) ? AMO_AND_D : AMO_AND_W;
            break;
        case 16:
            *op = (funct3 == 3) ? AMO_MIN_D : AMO_MIN_W;
            break;
        case 20:
            *op = (funct3 == 3) ? AMO_MAX_D : AMO_MAX_W;
            break;
        case 24:
            *op = (funct3 == 3) ? AMO_MINU_D : AMO_MINU_W;
            break;
        case 28:
            *op = (funct3 == 3) ? AMO_MAXU_D : AMO_MAXU_W;
            break;
        default:
            panic("%s: bad system instruction (0x%x)\n", __func__, inst);
        }

        break;

    case OP_FP:
        switch (funct7)
        {
        case 0x78:
            *op = FMV_W_X;
            break;
        default:
            panic("%s: bad fp instruction (0x%x)\n", __func__, inst);
        }

        break;

    default:
        panic("%s: bad instruction (0x%x) at (0x%lx)\n",
              __func__, inst, pc);
    }
}
