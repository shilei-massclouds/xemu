/*
 * Operation
 */

#ifndef OP_H
#define OP_H

typedef enum _op_t
{
    NOP = 0,

    LUI,

    AUIPC,

    JAL,

    JALR,

    BEQ,
    BNE,
    BLT,
    BGE,
    BLTU,
    BGEU,

    LB,
    LH,
    LW,
    LD,
    LBU,
    LHU,
    LWU,

    SB,
    SH,
    SW,
    SD,

    ADDI,
    SLLI,
    SLTI,
    SLTIU,
    XORI,
    SRLI,
    SRAI,
    ORI,
    ANDI,

    ADDIW,
    SLLIW,
    SRLIW,
    SRAIW,

    ADD,
    SUB,
    SLL,
    SLT,
    SLTU,
    XOR,
    SRL,
    SRA,
    OR,
    AND,

    ADDW,
    SUBW,
    SLLW,
    SRLW,
    SRAW,

    FENCE,
    FENCE_I,

    ECALL,
    EBREAK,
    XRET,
    WFI,

    CSRRW,
    CSRRS,
    CSRRC,

    CSRRWI,
    CSRRSI,
    CSRRCI,

    MUL,
    MULH,
    MULHSU,
    MULHU,
    DIV,
    DIVU,
    REM,
    REMU,

    MULW,
    DIVW,
    DIVUW,
    REMW,
    REMUW,

    AMO_ADD_D,
    AMO_SWAP_D,
    LR_D,
    SC_D,
    AMO_XOR_D,
    AMO_OR_D,
    AMO_AND_D,
    AMO_MIN_D,
    AMO_MAX_D,
    AMO_MINU_D,
    AMO_MAXU_D,

    AMO_ADD_W,
    AMO_SWAP_W,
    LR_W,
    SC_W,
    AMO_XOR_W,
    AMO_OR_W,
    AMO_AND_W,
    AMO_MIN_W,
    AMO_MAX_W,
    AMO_MINU_W,
    AMO_MAXU_W,

    OP_MAX_NUM,
} op_t;

const char *
op_name(op_t op);

#endif /* OP_H */
