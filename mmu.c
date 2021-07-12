/*
 * MMU
 */

#include "mmu.h"

#include "util.h"
#include "csr.h"

#define PTE_V(pte) BIT(pte, 0)
#define PTE_R(pte) BIT(pte, 1)
#define PTE_W(pte) BIT(pte, 2)
#define PTE_X(pte) BIT(pte, 3)


uint64_t
mmu(address_space *as, uint64_t vaddr, int *except)
{
    uint64_t pte;
    uint64_t root_ppn;
    uint64_t paddr;

    if ((priv != S_MODE) || (BITS(csr[SATP], 63, 60) == 0))
        return vaddr;

    root_ppn = BITS(csr[SATP], 43, 0);

    /* Level-2 */
    paddr = (root_ppn << 12) | (BITS(vaddr, 38, 30) << 3);
    pte = read_nommu(as, paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        *except = 1;
        return 0;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        return ((BITS(pte, 53, 28) << 30) | BITS(vaddr, 29, 0));
    }

    /* Level-1 */
    paddr = (BITS(pte, 53, 10) << 12) | (BITS(vaddr, 29, 21) << 3);
    pte = read_nommu(as, paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        *except = 1;
        return 0;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        return ((BITS(pte, 53, 19) << 21) | BITS(vaddr, 20, 0));
    }

    /* Level-0 */
    paddr = (BITS(pte, 53, 10) << 12) | (BITS(vaddr, 20, 12) << 3);
    pte = read_nommu(as, paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        *except = 1;
        return 0;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        return ((BITS(pte, 53, 10) << 12) | BITS(vaddr, 11, 0));
    }

    /* page-fault */
    *except = 1;
    return 0;
}
