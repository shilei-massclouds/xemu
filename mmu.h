/*
 * MMU
 */

#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "address_space.h"

int
mmu(address_space *as, uint64_t vaddr, uint64_t *paddr);

#endif /* MMU_H */
