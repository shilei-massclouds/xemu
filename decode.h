/*
 * Decode
 */

#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

#include "operation.h"

uint64_t
decode(uint64_t  pc,
       uint32_t  inst,
       op_t      *op,
       uint32_t  *rd,
       uint32_t  *rs1,
       uint32_t  *rs2,
       uint64_t  *imm,
       uint32_t  *csr_addr,
       uint32_t  *opcode);

void
dec32(uint64_t  pc,
      uint32_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr,
      uint32_t  *opcode);

void
dec16(uint64_t  pc,
      uint16_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr,
      uint32_t  *opcode);

#endif /* DECODE_H */
