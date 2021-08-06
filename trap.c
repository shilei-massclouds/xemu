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
        SET_BIT(sstatus, BIT_SPP_POS, mode_bit);
        priv = S_MODE;

        csr_update(SCAUSE, cause, CSR_OP_WRITE, &has_except);
        csr_update(STVAL, tval, CSR_OP_WRITE, &has_except);

        SET_BIT(sstatus, BIT_SPIE_POS, BIT(sstatus, BIT_SIE_POS));
        SET_BIT(sstatus, BIT_SIE_POS, 0UL);

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

        SET_BIT(mstatus, BIT_MPIE_POS, BIT(mstatus, BIT_MIE_POS));
        SET_BIT(mstatus, BIT_MIE_POS, 0UL);

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
        priv = BIT(sstatus, BIT_SPP_POS) ? S_MODE : U_MODE;
        SET_BIT(sstatus, BIT_SIE_POS, BIT(sstatus, BIT_SPIE_POS));
        csr_update(SSTATUS, sstatus, CSR_OP_WRITE, &has_except);
        ret = csr_read(SEPC, &has_except);
        break;

    case MRET:
        mstatus = csr_read(MSTATUS, &has_except);
        priv = BITS(mstatus, 12, 11);
        SET_BIT(mstatus, BIT_MIE_POS, BIT(mstatus, BIT_MPIE_POS));
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
    bool deleg = false;
    bool has_except = false;
    uint32_t irq = 0;
    uint32_t irq_bit = 0;
    uint64_t cause = 0;
    uint32_t eid = 0;
    intr_type_t type = INTR_TYPE_NONE;

    /* Source */
    eid = plic_interrupt();
    if (eid)
        type = EXTERNAL_INTR_TYPE;
    else
        type = clint_interrupt();

    if (!type)
        return pc;

    cause = intr_cause(type, priv);
    irq = cause & 0xF;

    irq_bit = (1UL << irq);

    /* Target */
    if (priv != M_MODE) {
        uint64_t mideleg = csr_read(MIDELEG, &has_except);
        deleg = mideleg & irq_bit;
    }

    bool enable = false;
    if (deleg) {
        uint64_t sstatus = csr_read(SSTATUS, &has_except);
        enable = sstatus & BIT_SIE;
        if (enable) {
            uint64_t sie = csr_read(SIE, &has_except);
            enable = sie & irq_bit;
        }
    } else {
        uint64_t mstatus = csr_read(MSTATUS, &has_except);
        enable = mstatus & BIT_MIE;
        if (enable) {
            uint64_t mie = csr_read(MIE, &has_except);
            enable = mie & irq_bit;
        }
    }

    if (enable) {
        if (deleg)
            csr_update(SIP, irq_bit, CSR_OP_SET, &has_except);
        else
            csr_update(MIP, irq_bit, CSR_OP_SET, &has_except);

        pc = trap_enter(pc, cause, 0);
    }

    return pc;
}
