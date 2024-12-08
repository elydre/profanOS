/*****************************************************************************\
|   === div.c : 2024 ===                                                      |
|                                                                             |
|    64-bit integer division and modulo functions                  .pi0iq.    |
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

uint64_t __udivmoddi4(uint64_t n, uint64_t d, uint64_t *rp) {
    uint64_t q = 0, r = n, y = d;
    uint32_t lz1, lz2, i, k;

    if (y <= r) {
        lz1 = __builtin_clzll(d); // count leading zeros
        lz2 = __builtin_clzll(n);

        k = lz1 - lz2;
        y = (y << k);

        if (r >= y) {
            r = r - y;
            q = (1ULL << k);
        }

        if (k > 0) {
            y = y >> 1;

            i = k;
            do {
                if (r >= y)
                    r = ((r - y) << 1) + 1;
                else
                    r = (r << 1);
                i = i - 1;
            } while (i != 0);

            q = q + r;
            r = r >> k;
            q = q - (r << k);
        }
    }

    if (rp)
        *rp = r;
    return q;
}

int64_t __divdi3(int64_t u, int64_t v) {
    int64union_t uu = {.ll = u};
    int64union_t vv = {.ll = v};
    int32_t c = 0;
    int64_t w;

    if (uu.s.high < 0)
        c = ~c, uu.ll = -uu.ll;

    if (vv.s.high < 0)
        c = ~c, vv.ll = -vv.ll;

    w = __udivmoddi4(uu.ll, vv.ll, NULL);

    if (c)
        w = -w;
    return w;
}

int64_t __moddi3(int64_t u, int64_t v) {
    int64union_t uu = {.ll = u};
    int64union_t vv = {.ll = v};
    int32_t c = 0;
    int64_t w;

    if (uu.s.high < 0)
        c = ~c, uu.ll = -uu.ll;

    if (vv.s.high < 0)
        vv.ll = -vv.ll;

    __udivmoddi4(uu.ll, vv.ll, (uint64_t*) &w);

    if (c)
        w = -w;
  return w;
}

int64_t __divmoddi4(int64_t u, int64_t v, int64_t *rp) {
    int64union_t uu = {.ll = u};
    int64union_t vv = {.ll = v};
    int32_t c1 = 0, c2 = 0;
    int64_t w, r;

    if (uu.s.high < 0)
        c1 = ~c1, c2 = ~c2, uu.ll = -uu.ll;

    if (vv.s.high < 0)
        c1 = ~c1, vv.ll = -vv.ll;

    w = __udivmoddi4(uu.ll, vv.ll, (uint64_t*) &r);

    if (c1) w = -w;
    if (c2) r = -r;

    *rp = r;
    return w;
}

uint64_t __umoddi3(uint64_t u, uint64_t v) {
    uint64_t w;
    __udivmoddi4(u, v, &w);

    return w;
}

uint64_t __udivdi3(uint64_t n, uint64_t d) {
  return __udivmoddi4(n, d, NULL);
}

#endif
