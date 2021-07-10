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
    case SATP:
        return "satp";
    case MTVEC:
        return "mtvec";
    case MVENDORID:
        return "mvendorid";
    case MARCHID:
        return "marchid";
    case MIMPID:
        return "mimpid";
    case MHARTID:
        return "mhartid";
    default:
        panic("%s: bad csr address 0x%x\n", __func__, csr_addr);
    }

    return "";
}
