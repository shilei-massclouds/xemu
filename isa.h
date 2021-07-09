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

#endif  /* _ISA_H */
