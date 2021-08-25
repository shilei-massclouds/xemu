/*
 * System map file operation
 */

#ifndef _SYSTEM_MAP_H_
#define _SYSTEM_MAP_H_

#include <stdint.h>

void
setup_system_map(void);

const char *
match_in_system_map(const char *name, uint64_t *paddr);

#endif /* _SYSTEM_MAP_H_ */
