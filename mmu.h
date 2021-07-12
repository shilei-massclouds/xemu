/*
 * MMU
 */

#ifndef MMU_H
#define MMU_H

#include <stdint.h>

#include "address_space.h"

uint64_t
mmu(address_space *as, uint64_t vaddr, int *except);

#endif /* MMU_H */
