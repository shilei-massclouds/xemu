/*
 * Util
 */

#ifndef UTIL_H
#define UTIL_H

/* n must be power of 2 */
#define ROUND_UP(x, n) (((x) + (n) - 1u) & ~((n) - 1u))

#define BITS(SRC, HIGH, LOW) \
    ((SRC >> LOW) & ((1UL << (HIGH - LOW + 1UL)) - 1UL))

#define BIT(SRC, INDEX) ((SRC >> INDEX) & 1UL)

#define BITS_IN_PLACE(SRC, HIGH, LOW) \
    ((((1UL << (HIGH - LOW + 1UL)) - 1UL) << LOW) & SRC)

#define EXPAND_BIT(SRC, INDEX, NUM) \
    (((SRC >> INDEX) & 1UL) ? ((1UL << NUM) - 1UL) : 0)

#define TO_WORD(n) \
    ((EXPAND_BIT((n), 31, 32) << 32) | BITS((n), 31, 0))

void panic(const char *msg, ...);

#endif /* UTIL_H */
