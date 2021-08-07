/*
 * CSR
 */

#include "csr.h"
#include "util.h"

uint32_t _priv = M_MODE;
uint64_t _csr[4096] = {0};

uint32_t
priv(void)
{
    return _priv;
}

void
switch_to(uint32_t new_priv)
{
    _priv = new_priv;
}

void
csr_init()
{
    _csr[MISA] = MISA_INIT_VAL;
}

const char *
csr_name(uint32_t csr_addr)
{
    switch (csr_addr)
    {
    case USTATUS:
        return "ustatus";
    case FCSR:
        return "fcsr";
    case STVEC:
        return "stvec";
    case SSCRATCH:
        return "sscratch";
    case SATP:
        return "satp";
    case MISA:
        return "misa";
    case MIE:
        return "mie";
    case MTVEC:
        return "mtvec";
    case MSCRATCH:
        return "mscratch";
    case MIP:
        return "mip";
    case PMPADDR0:
    case PMPADDR1:
    case PMPADDR2:
    case PMPADDR3:
    case PMPADDR4:
    case PMPADDR5:
    case PMPADDR6:
    case PMPADDR7:
    case PMPADDR8:
    case PMPADDR9:
    case PMPADDR10:
    case PMPADDR11:
    case PMPADDR12:
    case PMPADDR13:
    case PMPADDR14:
    case PMPADDR15:
        return "pmpaddr";
    case MVENDORID:
        return "mvendorid";
    case MARCHID:
        return "marchid";
    case MIMPID:
        return "mimpid";
    case MHARTID:
        return "mhartid";
    default:
        return "unknown";
    }

    return "";
}

static uint64_t
_read(uint32_t addr, bool *has_except)
{
    switch (addr)
    {
    /* 0x003 */
    case FCSR:
        return _csr[addr];

    /* 0x100 */
    case SSTATUS:
    /* 0x102 ~ 0x106 */
    case SEDELEG:
    case SIDELEG:
    case SIE:
    case STVEC:
        return _csr[addr];

    case SCOUNTEREN:
        return 0x7; /* Only support cycle, time and instret */

    /* 0x140 ~ 0x144 */
    case SSCRATCH:
    case SEPC:
    case SCAUSE:
    case STVAL:
    case SIP:
        return _csr[addr];

    /* 0x180 */
    case SATP:
        return _csr[addr];

    /* 0x300 ~ 0x306 */
    case MSTATUS:
    case MISA:
    case MEDELEG:
    case MIDELEG:
    case MIE:
    case MTVEC:
        return _csr[addr];

    case MCOUNTEREN:
        return 0x7; /* Only support cycle, time and instret */

    /* 0x340 ~ 0x344 */
    case MSCRATCH:
    case MEPC:
    case MCAUSE:
    case MTVAL:
    case MIP:
        return _csr[addr];

    /* 0x3a0 */
    case PMPCFG0:
        return _csr[addr];

    /* 0x3a2 */
    case PMPCFG2:
        return _csr[addr];

    /* 0x3b0 ~ 0x3bf */
    case PMPADDR0...PMPADDR15:
        return _csr[addr];

    /* 0xc00 ~ 0xc02 */
    case TIME:
        return cpu_read_rtc();

    case CYCLE:
    case INSTRET:
        return cpu_get_host_ticks();

    /* 0xf11 ~ 0xf14 */
    case MVENDORID:
    case MARCHID:
    case MIMPID:
    case MHARTID:
        return _csr[addr];

    case PMPADDR16...PMPADDR63:
    case MHPMCOUNTER3...MHPMCOUNTER31:
        if (has_except)
            *has_except = true;
        break;

    default:
        if (addr != 0)
            panic("%s: bad addr 0x%x\n", __func__, addr);
    }

    return 0;
}

uint64_t
csr_update(uint32_t addr, uint64_t data, csr_op_type type, bool *has_except)
{
    uint64_t ret;

    if (addr >= 4096)
        panic("%s: bad addr 0x%x\n", __func__, addr);

    ret = _read(addr, has_except);
    if (*has_except)
        return 0;

    if (addr == MSTATUS && (data & 0xF))
        printf("### %s: mstatus = %lx\n", __func__, data);

    switch (type)
    {
    case CSR_OP_WRITE:
        _csr[addr] = data;
        break;
    case CSR_OP_SET:
        _csr[addr] = ret | data;
        break;
    case CSR_OP_CLEAR:
        _csr[addr] = ret & ~data;
        break;
    default:
        panic("%s: bad csr op %d\n", __func__, type);
    }

    return ret;
}

uint64_t
csr_read(uint32_t addr, bool *has_except)
{
    if (addr >= 4096)
        panic("%s: bad addr 0x%x\n", __func__, addr);

    return _read(addr, has_except);
}
