/*
 * CSR
 */

#ifndef CSR_H
#define CSR_H

#include <stdint.h>

// 63:62 MXL (10: XLEN = 64)
#define MISA_MXL_64 0x8000000000000000

// 61:26 WIRI

// 25 Z Reserved
#define MISA_Z 0x0000000002000000

// 24 Y Reserved
#define MISA_Y 0x0000000001000000

// 23 X Non-standard extensions present
#define MISA_X 0x0000000000800000

// 22 W Reserved
#define MISA_W 0x0000000000400000

// 21 V Tentatively reserved for Vector
#define MISA_V 0x0000000000200000

// 20 U User mode implemented
#define MISA_U 0x0000000000100000

// 19 T Tentatively reserved for Transactional Memory extension
#define MISA_T 0x0000000000080000

// 18 S Supervisor mode implemented
#define MISA_S 0x0000000000040000

// 17 R Reserved
#define MISA_R 0x0000000000020000

// 16 Q Quad-precision floating-point extension
#define MISA_Q 0x0000000000010000

// 15 P Tentatively reserved for Packed-SIMD extension
#define MISA_P 0x0000000000008000

// 14 O Reserved
#define MISA_O 0x0000000000004000

// 13 N User-level interrupts supported
#define MISA_N 0x0000000000002000

// 12 M Integer Multiply/Divide extension
#define MISA_M 0x0000000000001000

// 11 L Tentatively reserved for Decimal Floating-Point extension
#define MISA_L 0x0000000000000800

// 10 K Reserved
#define MISA_K 0x0000000000000400

//  9 J Reserved
#define MISA_J 0x0000000000000200

//  8 I RV32I/64I/128I base ISA
#define MISA_I 0x0000000000000100

//  7 H Hypervisor mode implemented
#define MISA_H 0x0000000000000080

//  6 G Additional standard extensions present
#define MISA_G 0x0000000000000040

//  5 F Single-precision floating-point extension
#define MISA_F 0x0000000000000020

//  4 E RV32E base ISA
#define MISA_E 0x0000000000000010

//  3 D Double-precision floating-point extension
#define MISA_D 0x0000000000000008

//  2 C Compressed extension
#define MISA_C 0x0000000000000004

//  1 B Tentatively reserved for Bit operations extension
#define MISA_B 0x0000000000000002

//  0 A Atomic extension
#define MISA_A 0x0000000000000001

#define MISA_INIT_VAL \
    (MISA_MXL_64 | MISA_U | MISA_S | MISA_M | MISA_I | MISA_C | MISA_A)

/* [M|S]STATUS bits */
#define MS_UIE      0
#define MS_SIE      1
#define MS_MIE      3
#define MS_UPIE     4
#define MS_SPIE     5
#define MS_MPIE     7
#define MS_SPP      8

#define CAUSE_U_SOFTWARE_INTR  0x8000000000000000
#define CAUSE_S_SOFTWARE_INTR  0x8000000000000001
#define CAUSE_M_SOFTWARE_INTR  0x8000000000000003
#define CAUSE_U_TIMER_INTR     0x8000000000000004
#define CAUSE_S_TIMER_INTR     0x8000000000000005
#define CAUSE_M_TIMER_INTR     0x8000000000000007
#define CAUSE_U_EXTERNAL_INTR  0x8000000000000008
#define CAUSE_S_EXTERNAL_INTR  0x8000000000000009
#define CAUSE_M_EXTERNAL_INTR  0x800000000000000b

#define CAUSE_INST_ADDR_MISALIGNED  0x0
#define CAUSE_INST_ACCESS_FAULT     0x1
#define CAUSE_ILLEGAL_INST          0x2
#define CAUSE_BREAK_POINT           0x3
#define CAUSE_LOAD_ADDR_MISALIGNED  0x4
#define CAUSE_LOAD_ACCESS_FAULT     0x5
#define CAUSE_STORE_ADDR_MISALIGNED 0x6
#define CAUSE_STORE_ACCESS_FAULT    0x7
#define CAUSE_ECALL_FROM_U_MODE     0x8
#define CAUSE_ECALL_FROM_S_MODE     0x9
#define CAUSE_ECALL_FROM_M_MODE     0xb
#define CAUSE_INST_PAGE_FAULT       0xc
#define CAUSE_LOAD_PAGE_FAULT       0xd
#define CAUSE_STORE_PAGE_FAULT      0xf

/* CSR Number */
#define USTATUS     0x000

#define SSTATUS     0x100
#define SEDELEG     0x102
#define SIDELEG     0x103
#define SIE         0x104
#define STVEC       0x105
#define SCOUNTEREN  0x106

#define SSCRATCH    0x140
#define SEPC        0x141
#define SCAUSE      0x142
#define STVAL       0x143
#define SIP         0x144

#define SATP        0x180

#define MSTATUS     0x300
#define MISA        0x301
#define MEDELEG     0x302
#define MIDELEG     0x303
#define MIE         0x304
#define MTVEC       0x305
#define MCOUNTEREN  0x306

#define MSCRATCH    0x340
#define MEPC        0x341
#define MCAUSE      0x342
#define MTVAL       0x343
#define MIP         0x344

#define PMPCFG0     0x3a0
#define PMPCFG2     0x3a2
#define PMPADDR0    0x3b0
#define PMPADDR1    0x3b1
#define PMPADDR2    0x3b2
#define PMPADDR3    0x3b3
#define PMPADDR4    0x3b4
#define PMPADDR5    0x3b5
#define PMPADDR6    0x3b6
#define PMPADDR7    0x3b7
#define PMPADDR8    0x3b8
#define PMPADDR9    0x3b9
#define PMPADDR10   0x3ba
#define PMPADDR11   0x3bb
#define PMPADDR12   0x3bc
#define PMPADDR13   0x3bd
#define PMPADDR14   0x3be
#define PMPADDR15   0x3bf

#define MVENDORID   0xf11
#define MARCHID     0xf12
#define MIMPID      0xf13
#define MHARTID     0xf14

#define U_MODE  0
#define S_MODE  1
#define M_MODE  3

extern uint8_t  priv;

typedef enum _csr_op_type
{
    CSR_OP_WRITE = 0,
    CSR_OP_SET,
    CSR_OP_CLEAR,
} csr_op_type;

void
csr_init();

const char *
csr_name(uint32_t csr_addr);

uint64_t
csr_update(uint32_t addr, uint64_t data, csr_op_type type);

uint64_t
csr_read(uint32_t addr);

#endif /* CSR_H */
