/*
 * Regfile
 */

#ifndef REGFILE_H
#define REGFILE_H

#include <stdint.h>

extern uint64_t reg[32];

const char * reg_name(int index);

#endif /* REGFILE_H */
