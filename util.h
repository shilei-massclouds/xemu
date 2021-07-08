/*
 * Util
 */

#ifndef UTIL_H
#define UTIL_H

/* n must be power of 2 */
#define ROUND_UP(x, n) (((x) + (n) - 1u) & ~((n) - 1u))

void panic(const char *msg, ...);

#endif /* UTIL_H */
