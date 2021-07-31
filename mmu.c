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


int
mmu(address_space *as, uint64_t vaddr, uint64_t *paddr)
{
    uint64_t pte;
    uint64_t root_ppn;
    bool has_except = false;

    *paddr = 0;

    if ((priv == M_MODE) || (BITS(csr_read(SATP, &has_except), 63, 60) == 0)) {
        *paddr = vaddr;
        return 0;
    }

    root_ppn = BITS(csr_read(SATP, &has_except), 43, 0);

    /* Level-2 */
    *paddr = (root_ppn << 12) | (BITS(vaddr, 38, 30) << 3);
    pte = as_read_nommu(as, *paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        return -1;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        *paddr = ((BITS(pte, 53, 28) << 30) | BITS(vaddr, 29, 0));
        return 0;
    }

    /* Level-1 */
    *paddr = (BITS(pte, 53, 10) << 12) | (BITS(vaddr, 29, 21) << 3);
    pte = as_read_nommu(as, *paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        return -1;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        *paddr = ((BITS(pte, 53, 19) << 21) | BITS(vaddr, 20, 0));
        return 0;
    }

    /* Level-0 */
    *paddr = (BITS(pte, 53, 10) << 12) | (BITS(vaddr, 20, 12) << 3);
    pte = as_read_nommu(as, *paddr, 8, 0);

    if ((PTE_V(pte) == 0) || ((PTE_R(pte) == 0) && (PTE_W(pte) == 1))) {
        /* page-fault */
        return -1;
    }

    if (PTE_R(pte) || (PTE_X(pte))) {
        /* leaf */
        *paddr = ((BITS(pte, 53, 10) << 12) | BITS(vaddr, 11, 0));
        return 0;
    }

    /* page-fault */
    return -1;
}
