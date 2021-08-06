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
    uint32_t offset;

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

#endif /* INTERRUPT_H */
