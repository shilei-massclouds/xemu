/*
 * MMU
 */

#ifndef MMU_H
#define MMU_H

#include <stdint.h>

uint64_t
mmu(uint64_t vaddr, int *except);

#endif /* MMU_H */
