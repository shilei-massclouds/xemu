/*
 * Trap
 */

#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

#include "util.h"
#include "csr.h"
#include "operation.h"

uint64_t
trap_enter(uint64_t pc, uint64_t cause, uint64_t tval);

uint64_t
trap_exit(op_t op);

#endif /* TRAP_H */
