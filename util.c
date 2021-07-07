/*
 * Util
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

const char* _abi_names[32] = {
    "zero",                                                 /* 0 */
    "ra", "sp", "gp", "tp", "t0", "t1", "t2", "s0", "s1",   /* 1 ~ 9 */
    "a0", "a1", "a2", "a3", "a4", "a5", "a6", "a7",         /* 10 ~ 17 */
    "s2", "s3", "s4", "s5", "s6", "s7", "s8", "s9",         /* 18 ~ 25 */
    "s10", "s11", "t3", "t4", "t5", "t6"                    /* 26 ~ 31 */
};

const char *
reg_name(int index)
{
    return _abi_names[index];
}

void
panic(const char *msg, ...)
{
    va_list ap;
    printf("\n#############################\n");
    printf("PANIC: \n");
    va_start(ap, msg);
    vfprintf(stdout, msg, ap);
    va_end(ap);
    printf("System exit ...\n");
    printf("#############################\n\n");

    exit(-1);
}
