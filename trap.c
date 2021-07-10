/*
 * Trap
 */

#include "trap.h"

uint64_t
trap_enter(uint64_t pc, uint64_t cause, uint64_t tval)
{
    uint64_t ret;
    uint64_t mideleg = 0;
    uint64_t medeleg = 0;

    if (BIT(cause, 63))
        mideleg = BIT(csr[MIDELEG], BITS(cause, 3, 0));
    else
        medeleg = BIT(csr[MEDELEG], cause);

    if (mideleg || medeleg) {
        /* Handle trap in S_MODE */
        uint64_t mode_bit = (priv == U_MODE) ? 0UL : 1UL;
        SET_BIT(csr[SSTATUS], MS_SPP, mode_bit);
        priv = S_MODE;

        csr[SCAUSE] = cause;
        csr[STVAL] = tval;

        SET_BIT(csr[SSTATUS], MS_SPIE, BIT(csr[SSTATUS], MS_SIE));
        SET_BIT(csr[SSTATUS], MS_SIE, 0UL);

        csr[SEPC] = pc;
        ret = csr[STVEC];
    } else {
        /* Handle trap in M_MODE. MS_MPP at [12,11]. */
        SET_BITS(csr[MSTATUS], 12, 11, priv);
        priv = M_MODE;

        csr[MCAUSE] = cause;
        csr[MTVAL] = tval;

        SET_BIT(csr[MSTATUS], MS_MPIE, BIT(csr[MSTATUS], MS_MIE));
        SET_BIT(csr[MSTATUS], MS_MIE, 0UL);

        csr[MEPC] = pc;
        ret = csr[MTVEC];
    }

    return ret;
}
