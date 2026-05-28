/*****************************************************************************\
|   === popcount.c : 2026 ===                                                 |
|                                                                             |
|    Popcount functions for 32 and 64 bit integers                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from GCC libgcc2.c (gnu)                        `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../config_libc.h"

#if _JOIN_LIBCC_INSIDE

#include <stdint.h>
#include <stddef.h>

typedef union {
    struct {
        int32_t low, high;
    } s;
    int64_t ll;
} int64union_t;

const int __popcount_tab[256] = {
    0,1,1,2,1,2,2,3,1,2,2,3,2,3,3,4,1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    1,2,2,3,2,3,3,4,2,3,3,4,3,4,4,5,2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    2,3,3,4,3,4,4,5,3,4,4,5,4,5,5,6,3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,
    3,4,4,5,4,5,5,6,4,5,5,6,5,6,6,7,4,5,5,6,5,6,6,7,5,6,6,7,6,7,7,8
};

int __popcountdi2(uint64_t x) {
    int ret = 0;

    for (uint32_t i = 0; i < 2 * sizeof(uint32_t) * 8; i += 8)
        ret += __popcount_tab[(x >> i) & 0xff];

    return ret;
}

int __popcountsi2(uint32_t x) {
    int ret = 0;

    for (uint32_t i = 0; i < sizeof(uint32_t) * 8; i += 8)
        ret += __popcount_tab[(x >> i) & 0xff];

    return ret;
}

static inline int count_trailing_zeros(uint32_t word) {
    #if defined(__GNUC__) || defined(__clang__)
        return __builtin_ctz(word);
    #else
        int count = 0;
        while ((word & 1) == 0) {
            word >>= 1;
            count++;
        }
        return count;
    #endif
}

int __ffsdi2(uint64_t x) {
    const int64union_t uu = {.ll = x};
    uint32_t word, add;

    if (uu.s.low != 0)
        word = uu.s.low, add = 0;
    else if (uu.s.high != 0)
        word = uu.s.high, add = sizeof(uu.s.low) * 8;
    else
        return 0;

    return count_trailing_zeros(word) + add + 1;
}

int __ctzdi2(uint64_t x) {
    const int64union_t uu = {.ll = x};
    uint32_t word, add;

    if (uu.s.low)
        word = uu.s.low, add = 0;
    else
        word = uu.s.high, add = sizeof(uu.s.low) * 8;

    return count_trailing_zeros(word) + add;
}

#endif
