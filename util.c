/*
 * Util
 */

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <time.h>
#include <termio.h>
#include <sys/time.h>
#include <unistd.h>

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

int64_t
get_clock(void)
{
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec * 1000000000LL + ts.tv_nsec;
}

int64_t
get_clock_realtime(void)
{
    struct timeval tv;
    gettimeofday(&tv, NULL);
    return tv.tv_sec * 1000000000LL + (tv.tv_usec * 1000);
}

uint8_t
getch(void)
{
    char c;
    struct termios nts, ots;

    if (tcgetattr(STDIN_FILENO, &ots) < 0)
        return 0;

    nts = ots;
    cfmakeraw(&nts);

    if (tcsetattr(STDIN_FILENO, TCSANOW, &nts) < 0)
        return 0;

    c = getchar();

    if (tcsetattr(STDIN_FILENO, TCSANOW, &ots) < 0)
        return 0;

    return c;
}
