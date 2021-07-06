/*
 * Util
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "util.h"

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
