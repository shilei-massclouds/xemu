/*
 * Util
 */

#ifndef UTIL_H
#define UTIL_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>

#include "csr.h"

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
    ((EXPAND_BIT((uint64_t)(n), 31, 32) << 32) | BITS((uint64_t)(n), 31, 0))

#define _BITS_MASK(high, low) \
    (((1UL << ((high) - (low) + 1UL)) - 1UL) << (low))

#define SET_BITS(dest, high, low, bits) \
    (dest = (((dest) & ~_BITS_MASK((high), (low))) | ((bits) << (low))))

#define SET_BIT(dest, index, bit) \
    (dest = (((dest) & ~(1UL << (index))) | (bit << (index))))

/* Round number down to multiple */
#define ALIGN_DOWN(n, m) ((n) / (m) * (m))

/* Round number up to multiple. Safe when m is not a power of 2 (see
 * ROUND_UP for a faster version when a power of 2 is guaranteed) */
#define ALIGN_UP(n, m) ALIGN_DOWN((n) + (m) - 1, (m))

#define IO_VEC_F_READ   0x1
#define IO_VEC_F_WRITE  0x2

typedef struct _iovec_t
{
    uint64_t base;
    size_t   len;
    uint32_t flags;
} iovec_t;

void panic(const char *msg, ...);

/**
 * ctz32 - count trailing zeros in a 32-bit value.
 * @val: The value to search
 *
 * Returns 32 if the value is zero.  Note that the GCC builtin is
 * undefined if the value is zero.
 */
static inline int ctz32(uint32_t val)
{
    return val ? __builtin_ctz(val) : 32;
}

static inline int
get_file_size(const char *filename, uint64_t *psize)
{
    long size;
    FILE *fp = fopen(filename, "rb");
    if (fp == NULL)
        return -1;

    fseek(fp, 0, SEEK_END);
    size = ftell(fp);
    fclose(fp);

    if (size < 0)
        return -1;

    *psize = (uint64_t)size;
    return 0;
}

int64_t
get_clock(void);

int64_t
get_clock_realtime(void);

static inline int64_t
cpu_get_host_ticks(void)
{
    int64_t ret;
    uint32_t low;
    uint32_t high;

    asm volatile("rdtsc" : "=a"(low), "=d"(high));

    ret = high;
    ret <<= 32;
    ret |= low;

    return ret;
}

static inline void
_set_pending_bit(uint32_t dst, uint32_t src, uint32_t index)
{
    bool has_except = false;
    uint64_t data = csr_read(dst, &has_except);
    SET_BIT(data, index, BIT(csr_read(src, &has_except), index));
    csr_update(dst, data, CSR_OP_WRITE, &has_except);
}

void
cpu_enable_clock(void);

int64_t
cpu_get_clock(void);

uint64_t
cpu_read_rtc(void);

uint8_t
getch(void);

static inline bool
streq(const char *str, const char *val)
{
    return (strcmp(str, val) == 0);
}

#endif /* UTIL_H */
