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

    AMO_ADD,
    AMO_SWAP,
    LR,
    SC,
    AMO_XOR,
    AMO_OR,
    AMO_AND,
    AMO_MIN,
    AMO_MAX,
    AMO_MINU,
    AMO_MAXU,

    AMO_ADDW,
    AMO_SWAPW,
    LRW,
    SCW,
    AMO_XORW,
    AMO_ORW,
    AMO_ANDW,
    AMO_MINW,
    AMO_MAXW,
    AMO_MINUW,
    AMO_MAXUW,

    OP_MAX_NUM,
} op_t;

const char *
op_name(op_t op);

#endif /* OP_H */
