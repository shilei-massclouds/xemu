/*
 * CSR
 */

#include "csr.h"
#include "util.h"

uint8_t priv = M_MODE;
uint64_t _csr[4096] = {0};

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
_read(uint32_t addr)
{
    /*
    if (addr > PMPCFG2 && addr < PMPADDR0)
        panic("%s: illegal inst!\n", __func__);

    if (addr > PMPADDR15 && addr <= 0x400)
        panic("%s: illegal inst!\n", __func__);
    */

    switch (addr)
    {
    case CYCLE:
        return cpu_get_host_ticks();
    case TIME:
        return cpu_get_host_ticks();
    case INSTRET:
        return cpu_get_host_ticks();
    default:
        return _csr[addr];
    }

    return 0;
}

uint64_t
csr_update(uint32_t addr, uint64_t data, csr_op_type type)
{
    uint64_t ret;

    if (addr >= 4096)
        panic("%s: bad addr 0x%x\n", addr);

    ret = _read(addr);

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
csr_read(uint32_t addr)
{
    if (addr >= 4096)
        panic("%s: bad addr 0x%x\n", addr);

    return _read(addr);
}
