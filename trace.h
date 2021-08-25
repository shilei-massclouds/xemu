/*
 * Trace
 */

#ifndef _TRACE_H_
#define _TRACE_H_

#include <stdint.h>

#include "operation.h"

void
trace_decode(uint64_t   pc,
             op_t       op,
             uint32_t   rd,
             uint32_t   rs1,
             uint32_t   rs2,
             uint64_t   imm,
             uint32_t   csr_addr);

void
trace_execute(uint64_t   pc,
              op_t       op,
              uint32_t   rd,
              uint32_t   rs1,
              uint32_t   rs2,
              uint64_t   imm,
              uint32_t   csr_addr);

void
trace(uint64_t pc);

void
setup_trace_table(void);

#endif /* _TRACE_H_ */
