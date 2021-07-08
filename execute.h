/*
 * Execute
 */

#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>

#include "operation.h"

void
execute(op_t op,
        uint32_t rd, uint32_t rs1, uint32_t rs2,
        uint64_t imm, uint32_t csr_addr);

#endif /* EXECUTE_H */
