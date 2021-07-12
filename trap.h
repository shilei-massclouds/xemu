/*
 * Trap
 */

#ifndef TRAP_H
#define TRAP_H

#include <stdint.h>

#include "util.h"
#include "csr.h"

uint64_t
trap_enter(uint64_t pc, uint64_t cause, uint64_t tval);

uint64_t
trap_exit();

#endif /* TRAP_H */
