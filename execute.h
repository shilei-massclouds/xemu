/*
 * Execute
 */

#ifndef EXECUTE_H
#define EXECUTE_H

#include <stdint.h>

#include "operation.h"
#include "address_space.h"

uint64_t
execute(address_space *as,
        uint64_t pc, uint64_t next_pc,
        op_t op,
        uint32_t rd, uint32_t rs1, uint32_t rs2,
        uint64_t imm, uint32_t csr_addr);

#endif /* EXECUTE_H */
