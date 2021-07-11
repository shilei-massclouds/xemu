/*
 * Util
 */

#ifndef UTIL_H
#define UTIL_H


#define PAGE_SIZE 4096
#define PAGE_BITS 12

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

#define _BITS_MASK(high, low) \
    (((1UL << ((high) - (low) + 1UL)) - 1UL) << (low))

#define SET_BITS(dest, high, low, bits) \
    (dest = (((dest) & ~_BITS_MASK((high), (low))) | ((bits) << (low))))

#define SET_BIT(dest, index, bit) \
    (dest = (((dest) & ~(1UL << (index))) | (bit << (index))))

void panic(const char *msg, ...);

#endif /* UTIL_H */
