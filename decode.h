/*
 * Decode
 */

#ifndef DECODE_H
#define DECODE_H

#include <stdint.h>

#include "operation.h"

uint64_t
decode(uint32_t  inst,
       op_t      *op,
       uint32_t  *rd,
       uint32_t  *rs1,
       uint32_t  *rs2,
       uint64_t  *imm,
       uint32_t  *csr_addr);

void
dec32(uint32_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr);

void
dec16(uint16_t  inst,
      op_t      *op,
      uint32_t  *rd,
      uint32_t  *rs1,
      uint32_t  *rs2,
      uint64_t  *imm,
      uint32_t  *csr_addr);

#endif /* DECODE_H */
