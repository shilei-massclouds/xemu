/*
 * Interrupt
 */

#ifndef INTERRUPT_H
#define INTERRUPT_H

#include "util.h"

typedef enum _intr_type_t
{
    INTR_TYPE_NONE = 0,

    SOFTWARE_INTR_TYPE,
    TIMER_INTR_TYPE,
    EXTERNAL_INTR_TYPE,

    INTR_TYPE_LIMIT,
} intr_type_t;


static inline uint64_t
intr_cause(intr_type_t type, uint32_t priv)
{
    uint64_t offset;

    if (!type || type >= INTR_TYPE_LIMIT)
        panic("%s: bad type (%u)\n", __func__, type);

    offset = (type - 1) << 2;

    switch (priv)
    {
    case U_MODE:
        return CAUSE_U_SOFTWARE_INTR + offset;
    case S_MODE:
        return CAUSE_S_SOFTWARE_INTR + offset;
    case M_MODE:
        return CAUSE_M_SOFTWARE_INTR + offset;
    }

    return 0;
}

static inline uint32_t
intr_bit_flag(intr_type_t type, uint32_t priv)
{
    uint32_t irq = intr_cause(type, priv) & 0xF;
    return (1 << irq);
}

static inline uint32_t
intr_next_priv(intr_type_t type, uint32_t priv)
{
    if (priv != M_MODE) {
        uint64_t mideleg = csr_read(MIDELEG, NULL);
        uint32_t irq_bit = intr_bit_flag(type, S_MODE);
        if (mideleg & irq_bit)
            return S_MODE;
    }

    return M_MODE;
}

#endif /* INTERRUPT_H */
