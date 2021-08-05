/*
 * Trap
 */

#include "trap.h"
#include "device.h"

uint64_t
trap_enter(uint64_t pc, uint64_t cause, uint64_t tval)
{
    uint64_t ret;
    uint64_t mideleg = 0;
    uint64_t medeleg = 0;
    bool has_except = false;

    if (BIT(cause, 63))
        mideleg = BIT(csr_read(MIDELEG, &has_except), BITS(cause, 3, 0));
    else
        medeleg = BIT(csr_read(MEDELEG, &has_except), cause);

    if (mideleg || medeleg) {
        /* Handle trap in S_MODE */
        uint64_t mode_bit = (priv == U_MODE) ? 0UL : 1UL;
        uint64_t sstatus = csr_read(SSTATUS, &has_except);
        SET_BIT(sstatus, MS_SPP, mode_bit);
        priv = S_MODE;

        csr_update(SCAUSE, cause, CSR_OP_WRITE, &has_except);
        csr_update(STVAL, tval, CSR_OP_WRITE, &has_except);

        SET_BIT(sstatus, MS_SPIE, BIT(sstatus, MS_SIE));
        SET_BIT(sstatus, MS_SIE, 0UL);

        csr_update(SSTATUS, sstatus, CSR_OP_WRITE, &has_except);

        csr_update(SEPC, pc, CSR_OP_WRITE, &has_except);
        ret = csr_read(STVEC, &has_except);
    } else {
        /* Handle trap in M_MODE. MS_MPP at [12,11]. */
        uint64_t mstatus = csr_read(MSTATUS, &has_except);
        SET_BITS(mstatus, 12, 11, priv);
        priv = M_MODE;

        csr_update(MCAUSE, cause, CSR_OP_WRITE, &has_except);
        csr_update(MTVAL, tval, CSR_OP_WRITE, &has_except);

        SET_BIT(mstatus, MS_MPIE, BIT(mstatus, MS_MIE));
        SET_BIT(mstatus, MS_MIE, 0UL);

        csr_update(MSTATUS, mstatus, CSR_OP_WRITE, &has_except);

        csr_update(MEPC, pc, CSR_OP_WRITE, &has_except);
        ret = csr_read(MTVEC, &has_except);
    }

    return ret;
}

uint64_t
trap_exit(op_t op)
{
    uint64_t ret;
    uint64_t sstatus;
    uint64_t mstatus;
    bool has_except = false;

    switch (op)
    {
    case SRET:
        sstatus = csr_read(SSTATUS, &has_except);
        priv = BIT(sstatus, MS_SPP) ? S_MODE : U_MODE;
        SET_BIT(sstatus, MS_SIE, BIT(sstatus, MS_SPIE));
        csr_update(SSTATUS, sstatus, CSR_OP_WRITE, &has_except);
        ret = csr_read(SEPC, &has_except);
        break;

    case MRET:
        mstatus = csr_read(MSTATUS, &has_except);
        priv = BITS(mstatus, 12, 11);
        SET_BIT(mstatus, MS_MIE, BIT(mstatus, MS_MPIE));
        csr_update(MSTATUS, mstatus, CSR_OP_WRITE, &has_except);
        ret = csr_read(MEPC, &has_except);
        break;

    default:
        panic("%s: bad op(0x%x)\n", __func__, op);
    }

    return ret;
}

uint64_t
handle_interrupt(uint64_t pc)
{
    bool has_except = false;
    bool deleg = BIT(csr_read(MIDELEG, &has_except), 9);

    if (deleg) {
        if (!BIT(csr_read(SSTATUS, &has_except), MS_SIE))
            return pc;
    } else {
        if (!BIT(csr_read(MSTATUS, &has_except), MS_MIE))
            return pc;
    }

    if (plic_interrupt(deleg)) {
        uint64_t cause;
        //printf("%s: (%lx) 1\n", __func__, pc);

        if (deleg) {
            if (priv == S_MODE)
                _set_pending_bit(SIP, SIE, 9);
            else if (priv == U_MODE)
                _set_pending_bit(SIP, SIE, 8);
        } else {
            if (priv == S_MODE)
                _set_pending_bit(MIP, MIE, 9);
            else if (priv == U_MODE)
                _set_pending_bit(MIP, MIE, 8);
            else if (priv == M_MODE)
                _set_pending_bit(MIP, MIE, 11);
        }

        if (priv == S_MODE)
            cause = CAUSE_S_EXTERNAL_INTR;
        else if (priv == M_MODE)
            cause = CAUSE_M_EXTERNAL_INTR;
        else
            cause = CAUSE_U_EXTERNAL_INTR;

        pc = trap_enter(pc, cause, 0);
    } else if (check_clint()) {
        uint64_t cause;
        uint64_t newmip = csr_read(MIP, &has_except);
        uint64_t newmie = csr_read(MIE, &has_except);
        /*
        printf("%s: (%lx) 2 (%lx, %lx) %u\n",
               __func__, pc, newmip, newmie, priv);
               */

        _set_pending_bit(MIP, MIE, 7);
        cause = CAUSE_M_TIMER_INTR;
        pc = trap_enter(pc, cause, 0);
    } else {
        static uint64_t mip;
        uint64_t newmip = csr_read(MIP, &has_except);
        /*
        if (newmip != mip)
            printf("%s: (%lx) 3 (%lx, %lx) %u\n",
                   __func__, pc, newmip, mip, priv);
                   */

        mip = newmip;
        if ((mip & 0x20) && (priv == S_MODE)) {
            /*
            printf("%s: (%lx) 5 (%lx, %lx) %u\n",
                   __func__, pc, newmip, mip, priv);
                   */

            if (BIT(csr_read(SSTATUS, &has_except), MS_SIE) &&
                BIT(csr_read(SIE, &has_except), 5)) {
                uint64_t cause;
                _set_pending_bit(SIP, SIE, 5);
                cause = CAUSE_S_TIMER_INTR;
                pc = trap_enter(pc, cause, 0);
            }
        }
    }

    return pc;
}
