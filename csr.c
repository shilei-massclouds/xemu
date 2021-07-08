/*
 * CSR
 */

#include "csr.h"
#include "util.h"

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
    case 0xF11:
        return "mvendorid";
    case 0xF12:
        return "marchid";
    case 0xF13:
        return "mimpid";
    case 0xF14:
        return "mhartid";
    default:
        panic("%s: bad csr address 0x%x\n", __func__, csr_addr);
    }

    return "";
}
