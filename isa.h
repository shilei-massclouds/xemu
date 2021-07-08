/*
 * ISA
 */

#ifndef _ISA_H
#define _ISA_H

#define OP_NOP      0x00
#define OP_LOAD     0x03
#define OP_MISC     0x0F
#define OP_IMM      0x13
#define OP_AUIPC    0x17
#define OP_IMM_W    0x1B
#define OP_STORE    0x23
#define OP_AMO      0x2F
#define OP_REG      0x33
#define OP_LUI      0x37
#define OP_REG_W    0x3B
#define OP_BRANCH   0x63
#define OP_JALR     0x67
#define OP_JAL      0x6F
#define OP_SYSTEM   0x73

/* XLEN refers to the width of an X register
   in bits (32 or 64) */
#define XLEN        64
#define XMSB        (`XLEN - 1)

/* In ALU, operand extends with a bit 'CF' */
#define ALU_WIDTH   (`XLEN + 1)
#define ALU_MSB     `XLEN

#define TL_PUT_F            0x0
#define TL_PUT_P            0x1
#define TL_ARITH_DATA       0x2
#define TL_LOGIC_DATA       0x3
#define TL_GET              0x4

#define TL_ACCESS_ACK       0x0
#define TL_ACCESS_ACK_DATA  0x1

#define TL_PARAM_MIN    0x0
#define TL_PARAM_MAX    0x1
#define TL_PARAM_MINU   0x2
#define TL_PARAM_MAXU   0x3
#define TL_PARAM_ADD    0x4

#define TL_PARAM_XOR    0x0
#define TL_PARAM_OR     0x1
#define TL_PARAM_AND    0x2
#define TL_PARAM_SWAP   0x3

#endif  /* _ISA_H */
