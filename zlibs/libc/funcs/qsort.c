/*****************************************************************************\
|   === qsort.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of the qsort (stdlib.h) function from libC     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Original code from freeBSD (see below)                        `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

/* == Copyright (c) 1992, 1993
 * The Regents of the University of California. All rights reserved.
 *
 * Qsort routine from Bentley & McIlroy's "Engineering a Sort Function"
 *
 * https://github.com/freebsd/freebsd-src - libc/stdlib/qsort.c
 */

#include <sys/types.h>
#include <stdlib.h>

typedef int cmp_t(const void *, const void *);

#define MIN(a, b) ((a) < (b) ? a : b)

static inline void swapfunc(char *a, char *b, size_t es) {
    char t;

    do {
        t = *a;
        *a++ = *b;
        *b++ = t;
    } while (--es > 0);
}

#define vecswap(a, b, n) if ((n) > 0) swapfunc(a, b, n)
#define cmp(x, y) (cmp((x), (y)))

static inline char *med3(char *a, char *b, char *c, cmp_t *cmp) {
    return cmp(a, b) < 0 ?
        (cmp(b, c) < 0 ? b : (cmp(a, c) < 0 ? c : a )) :
        (cmp(b, c) > 0 ? b : (cmp(a, c) < 0 ? a : c ));
}

void qsort(void *a, size_t n, size_t es, cmp_t *cmp) {
    char *pa, *pb, *pc, *pd, *pl, *pm, *pn;
    size_t d1, d2;
    int cmp_result;
    int swap_cnt;

    if (n < 2)
        return;

    loop:
    swap_cnt = 0;

    if (n < 7) {
        for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
            for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0; pl -= es)
                swapfunc(pl, pl - es, es);
        return;
    }

    pm = (char *)a + (n / 2) * es;
    if (n > 7) {
        pl = a;
        pn = (char *) a + (n - 1) * es;
        if (n > 40) {
            size_t d = (n / 8) * es;

            pl = med3(pl, pl + d, pl + 2 * d, cmp);
            pm = med3(pm - d, pm, pm + d, cmp);
            pn = med3(pn - 2 * d, pn - d, pn, cmp);
        }
        pm = med3(pl, pm, pn, cmp);
    }
    swapfunc(a, pm, es);
    pa = pb = (char *)a + es;

    pc = pd = (char *)a + (n - 1) * es;
    for (;;) {
        while (pb <= pc && (cmp_result = cmp(pb, a)) <= 0) {
            if (cmp_result == 0) {
                swap_cnt = 1;
                swapfunc(pa, pb, es);
                pa += es;
            }
            pb += es;
        }
        while (pb <= pc && (cmp_result = cmp(pc, a)) >= 0) {
            if (cmp_result == 0) {
                swap_cnt = 1;
                swapfunc(pc, pd, es);
                pd -= es;
            }
            pc -= es;
        }
        if (pb > pc)
            break;
        swapfunc(pb, pc, es);
        swap_cnt = 1;
        pb += es;
        pc -= es;
    }

    if (swap_cnt == 0) {  // Switch to insertion sort
        for (pm = (char *) a + es; pm < (char *) a + n * es; pm += es)
            for (pl = pm; pl > (char *) a && cmp(pl - es, pl) > 0; pl -= es)
                swapfunc(pl, pl - es, es);
        return;
    }

    pn = (char *)a + n * es;
    d1 = MIN(pa - (char *) a, pb - pa);
    vecswap(a, pb - d1, d1);

    d1 = MIN(pd - pc, pn - pd - (ssize_t) es);
    vecswap(pb, pn - d1, d1);

    d1 = pb - pa;
    d2 = pd - pc;
    if (d1 <= d2) {
        // Recurse on left partition, then iterate on right partition
        if (d1 > es)
            qsort(a, d1 / es, es, cmp);
        if (d2 > es) {
            // Iterate rather than recurse to save stack space
            // qsort(pn - d2, d2 / es, es, cmp);
            a = pn - d2;
            n = d2 / es;
            goto loop;
        }
    } else {
        // Recurse on right partition, then iterate on left partition
        if (d2 > es)
            qsort(pn - d2, d2 / es, es, cmp);
        if (d1 > es) {
            // Iterate rather than recurse to save stack space
            // qsort(a, d1 / es, es, cmp);
            n = d1 / es;
            goto loop;
        }
    }
}
