/*
 * CSR
 */

#include "csr.h"
#include "util.h"

uint8_t priv = M_MODE;
uint64_t csr[4096] = {0};

void
csr_init()
{
    csr[MISA] = MISA_INIT_VAL;
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
        //panic("%s: bad csr address 0x%x\n", __func__, csr_addr);
    }

    return "";
}
