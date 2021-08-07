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
trap_enter(uint64_t pc, uint32_t next_priv, uint64_t cause, uint64_t tval);

uint64_t
trap_exit(op_t op);

uint64_t
handle_interrupt(uint64_t pc);

static inline uint32_t
except_bit_flag(uint64_t cause)
{
    return (1U << cause);
}

static inline uint32_t
except_next_priv(uint64_t cause, uint32_t priv)
{
    if (priv != M_MODE) {
        uint64_t medeleg = csr_read(MEDELEG, NULL);
        uint32_t except_bit = except_bit_flag(cause);
        if (medeleg & except_bit)
            return S_MODE;
    }

    return M_MODE;
}

static inline uint64_t
raise_except(uint64_t pc, uint64_t cause, uint64_t tval)
{
    return trap_enter(pc, except_next_priv(cause, priv()), cause, tval);
}

#endif /* TRAP_H */
