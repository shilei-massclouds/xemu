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
    fprintf(stderr, "\n#############################\n");
    fprintf(stderr, "PANIC:\n");
    va_start(ap, msg);
    vfprintf(stderr, msg, ap);
    va_end(ap);
    fprintf(stderr, "System exit ...\n");
    fprintf(stderr, "#############################\n\n");

    exit(-1);
}
