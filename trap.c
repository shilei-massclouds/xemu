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
        mideleg = BIT(csr_read(MIDELEG), BITS(cause, 3, 0));
    else
        medeleg = BIT(csr_read(MEDELEG), cause);

    if (mideleg || medeleg) {
        /* Handle trap in S_MODE */
        uint64_t mode_bit = (priv == U_MODE) ? 0UL : 1UL;
        uint64_t sstatus = csr_read(SSTATUS);
        SET_BIT(sstatus, MS_SPP, mode_bit);
        priv = S_MODE;

        csr_update(SCAUSE, cause, CSR_OP_WRITE);
        csr_update(STVAL, tval, CSR_OP_WRITE);

        SET_BIT(sstatus, MS_SPIE, BIT(sstatus, MS_SIE));
        SET_BIT(sstatus, MS_SIE, 0UL);

        csr_update(SSTATUS, sstatus, CSR_OP_WRITE);

        csr_update(SEPC, pc, CSR_OP_WRITE);
        ret = csr_read(STVEC);
    } else {
        /* Handle trap in M_MODE. MS_MPP at [12,11]. */
        uint64_t mstatus = csr_read(MSTATUS);
        SET_BITS(mstatus, 12, 11, priv);
        priv = M_MODE;

        csr_update(MCAUSE, cause, CSR_OP_WRITE);
        csr_update(MTVAL, tval, CSR_OP_WRITE);

        SET_BIT(mstatus, MS_MPIE, BIT(mstatus, MS_MIE));
        SET_BIT(mstatus, MS_MIE, 0UL);

        csr_update(MSTATUS, mstatus, CSR_OP_WRITE);

        csr_update(MEPC, pc, CSR_OP_WRITE);
        ret = csr_read(MTVEC);
    }

    return ret;
}

uint64_t
trap_exit(op_t op)
{
    uint64_t ret;
    uint64_t sstatus;
    uint64_t mstatus;

    switch (op)
    {
    case SRET:
        sstatus = csr_read(SSTATUS);
        priv = BIT(sstatus, MS_SPP) ? S_MODE : U_MODE;
        SET_BIT(sstatus, MS_SIE, BIT(sstatus, MS_SPIE));
        csr_update(SSTATUS, sstatus, CSR_OP_WRITE);
        ret = csr_read(SEPC);
        break;

    case MRET:
        mstatus = csr_read(MSTATUS);
        priv = BITS(mstatus, 12, 11);
        SET_BIT(mstatus, MS_MIE, BIT(mstatus, MS_MPIE));
        csr_update(MSTATUS, mstatus, CSR_OP_WRITE);
        ret = csr_read(MEPC);
        break;

    default:
        panic("%s: bad op(0x%x)\n", __func__, op);
    }

    return ret;
}
