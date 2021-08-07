/*
 * Decode 16-bit instruction
 */

#include <stdint.h>

#include "isa.h"
#include "operation.h"
#include "util.h"


#define N_IMM(INST) (((uint32_t)BITS(INST, 10, 7) << 6) | \
                     ((uint32_t)BITS(INST, 12, 11) << 4) | \
                     (BIT(INST, 5) << 3) | \
                     (BIT(INST, 6) << 2))

#define U_IMM(INST) ((BIT(INST, 5) << 6) | \
                     ((uint32_t)BITS(INST, 12, 10) << 3) | \
                     (BIT(INST, 6) << 2))

#define U_IMM_D(INST) (((uint32_t)BITS(INST, 6, 5) << 6) | \
                       ((uint32_t)BITS(INST, 12, 10) << 3))

#define I_IMM(INST) ((EXPAND_BIT(INST, 12, 59) << 5) | (uint32_t)BITS(INST, 6, 2))

#define D_IMM(INST) ((EXPAND_BIT(INST, 12, 55) << 9) | \
                     ((uint32_t)BITS(INST, 4, 3) << 7) | \
                     (BIT(INST, 5) << 6) | \
                     (BIT(INST, 2) << 5) | \
                     (BIT(INST, 6) << 4))

#define UI_IMM(INST) ((BIT(INST, 12) << 5) | (uint32_t)BITS(INST, 6, 2))

#define J_IMM(INST) ((EXPAND_BIT(INST, 12, 53) << 11) | \
                     (BIT(INST, 8) << 10) | \
                     ((uint32_t)BITS(INST, 10, 9) << 8) |\
                     (BIT(INST, 6) << 7) | \
                     (BIT(INST, 7) << 6) | \
                     (BIT(INST, 2) << 5) | \
                     (BIT(INST, 11) << 4) | \
                     ((uint32_t)BITS(INST, 5, 3) << 1))

#define B_IMM(INST) ((EXPAND_BIT(INST, 12, 56) << 8) | \
                     ((uint32_t)BITS(INST, 6, 5) << 6) |\
                     (BIT(INST, 2) << 5) | \
                     ((uint32_t)BITS(INST, 11, 10) << 3) |\
                     ((uint32_t)BITS(INST, 4, 3) << 1))

#define LWSP_IMM(INST) (((uint32_t)BITS(INST, 3, 2) << 6) | \
                        (BIT(INST, 12) << 5) | \
                        ((uint32_t)BITS(INST, 6, 4) << 2))

#define LDSP_IMM(INST) (((uint32_t)BITS(INST, 4, 2) << 6) | \
                        (BIT(INST, 12) << 5) | \
                        ((uint32_t)BITS(INST, 6, 5) << 3))

#define SWSP_IMM(INST) (((uint32_t)BITS(INST, 8, 7) << 6) | \
                        ((uint32_t)BITS(INST, 12, 9) << 2))

#define SDSP_IMM(INST) (((uint32_t)BITS(INST, 9, 7) << 6) | \
                        ((uint32_t)BITS(INST, 12, 10) << 3))


void
dec16(uint64_t  pc,
      uint16_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr)
{
    uint32_t opcode = (uint32_t)BITS(inst, 1, 0);
    uint32_t funct3 = (uint32_t)BITS(inst, 15, 13);

    *op = NOP;
    *rd = 0;
    *rs1 = 0;
    *rs2 = 0;
    *imm = 0;
    *csr_addr = 0;

    switch (opcode)
    {
    case 0:
        switch (funct3)
        {
        case 0:
            /* c.addi4spn */
            *op = ADDI;
            *rd = 8 + (uint32_t)(uint32_t)BITS(inst, 4, 2);
            *rs1 = 2;   /* sp */
            *imm = N_IMM(inst);
            break;

        case 1:
            /* c.fld */
            *op = FLD;
            *rd = 8 + (uint32_t)(uint32_t)BITS(inst, 4, 2);
            *rs1 = 8 + (uint32_t)(uint32_t)BITS(inst, 9, 7);
            *imm = U_IMM_D(inst);
            break;

        case 2:
            /* c.lw */
            *op = LW;
            *rd = 8 + (uint32_t)(uint32_t)BITS(inst, 4, 2);
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *imm = U_IMM(inst);
            break;

        case 3:
            /* c.ld */
            *op = LD;
            *rd = 8 + (uint32_t)BITS(inst, 4, 2);
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *imm = U_IMM_D(inst);
            break;

        case 5:
            /* c.fsd */
            *op = FSD;
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
            *imm = U_IMM_D(inst);
            break;

        case 6:
            /* c.sw */
            *op = SW;
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
            *imm = U_IMM(inst);
            break;

        case 7:
            /* c.sd */
            *op = SD;
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
            *imm = U_IMM_D(inst);
            break;

        default:
            panic("%s: bad instruction (0x%x)\n", __func__, inst);
        }

        break;

    case 1:
        switch (funct3)
        {
        case 0:
            /* c.nop | c.addi */
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = *rd;
            *op = *rd ? ADDI : NOP;
            *imm = *rd ? I_IMM(inst) : 0;
            break;

        case 1:
            /* c.addiw */
            *op = ADDIW;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = *rd;
            *imm = I_IMM(inst);
            break;

        case 2:
            /* c.li */
            *op = ADDI;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *imm = I_IMM(inst);
            break;

        case 3:
            /* c.addi16sp | c.lui */
            *rd = (uint32_t)BITS(inst, 11, 7);
            if (*rd == 2) {
                *op = ADDI;
                *rs1 = 2;
                *imm = D_IMM(inst);
            } else {
                *op = LUI;
                *imm = I_IMM(inst) << 12;
            }
            break;

        case 4:
            /* c.srli | c.srai | c.andi | (c.arith) */
            switch ((uint32_t)BITS(inst, 11, 10))
            {
            case 0:
                /* c.srli */
                *op = SRLI;
                *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                *rs1 = *rd;
                *imm = UI_IMM(inst);
                break;

            case 1:
                /* c.srai */
                *op = SRAI;
                *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                *rs1 = *rd;
                *imm = UI_IMM(inst);
                break;

            case 2:
                /* c.andi */
                *op = ANDI;
                *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                *rs1 = *rd;
                *imm = I_IMM(inst);
                break;

            case 3:
                switch ((uint32_t)BITS(inst, 6, 5))
                {
                case 0:
                    /* c.sub | c.subw */
                    *op = BIT(inst, 12) ? SUBW : SUB;
                    *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                    *rs1 = *rd;
                    *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
                    break;

                case 1:
                    /* c.xor | c.addw */
                    *op = BIT(inst, 12) ? ADDW : XOR;
                    *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                    *rs1 = *rd;
                    *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
                    break;

                case 2:
                    /* c.or */
                    *op = OR;
                    *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                    *rs1 = *rd;
                    *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
                    break;

                case 3:
                    /* c.and */
                    *op = AND;
                    *rd = 8 + (uint32_t)BITS(inst, 9, 7);
                    *rs1 = *rd;
                    *rs2 = 8 + (uint32_t)BITS(inst, 4, 2);
                    break;

                default:
                    panic("%s: bad instruction (0x%x)", __func__, inst);
                }
                break;

            default:
                panic("%s: bad instruction (0x%x)", __func__, inst);
            }
            break;

        case 5:
            /* c.j */
            *op = JAL;
            *imm = J_IMM(inst);
            break;

        case 6:
            /* c.beqz */
            *op = BEQ;
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *imm = B_IMM(inst);
            break;

        case 7:
            /* c.bnez */
            *op = BNE;
            *rs1 = 8 + (uint32_t)BITS(inst, 9, 7);
            *imm = B_IMM(inst);
            break;

        default:
            panic("%s: bad instruction (0x%x)", __func__, inst);
        }

        break;

    case 2:
        switch (funct3)
        {
        case 0:
            /* c.slli */
            *op = SLLI;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = *rd;
            *imm = UI_IMM(inst);
            break;

        case 1:
            /* c.fldsp */
            *op = FLD;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = 2;   /* sp */
            *imm = LDSP_IMM(inst);
            break;

        case 2:
            /* c.lwsp */
            *op = LW;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = 2;   /* sp */
            *imm = LWSP_IMM(inst);
            break;

        case 3:
            /* c.ldsp */
            *op = LD;
            *rd = (uint32_t)BITS(inst, 11, 7);
            *rs1 = 2;   /* sp */
            *imm = LDSP_IMM(inst);
            break;

        case 4:
            if (BIT(inst, 12)) {
                *rs1 = (uint32_t)BITS(inst, 11, 7);
                if (*rs1) {
                    *rs2 = (uint32_t)BITS(inst, 6, 2);
                    if (*rs2) {
                        /* c.add */
                        *rd = *rs1;
                        *op = ADD;
                    } else {
                        /* c.jalr */
                        *rd = 1;    /* ra */
                        *op = JALR;
                    }
                } else {
                    /* c.ebreak */
                    *op = EBREAK;
                }
            } else {
                *rs2 = (uint32_t)BITS(inst, 6, 2);
                if (*rs2) {
                    /* c.mv */
                    *op = ADD;
                    *rd = (uint32_t)BITS(inst, 11, 7);
                } else {
                    /* c.jr */
                    *op = JALR;
                    *rs1 = (uint32_t)BITS(inst, 11, 7);
                }
            }
            break;

        case 6:
            /* c.swsp */
            *op = SW;
            *rs1 = 2;   /* sp */
            *rs2 = (uint32_t)BITS(inst, 6, 2);
            *imm = SWSP_IMM(inst);
            break;

        case 7:
            /* c.sdsp */
            *op = SD;
            *rs1 = 2;   /* sp */
            *rs2 = (uint32_t)BITS(inst, 6, 2);
            *imm = SDSP_IMM(inst);
            break;

        default:
            panic("%s: bad instruction (0x%x)\n", __func__, inst);
        }

        break;

    default:
        panic("%s: bad instruction (0x%x) at (0x%lx)\n",
              __func__, inst, pc);
    }
}
