/*****************************************************************************\
|   === wchar.c : 2025 ===                                                    |
|                                                                             |
|    Implementation of wchar functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS - under GPLv3                  q. /|\  "   |
|    Based on the musl libc implementation                         `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan.h>
#include <stdlib.h>
#include <wchar.h>
#include <errno.h>

// bittab for UTF-8 conversion
static const uint32_t bittab[] = {
    0xc0000002, 0xc0000003, 0xc0000004, 0xc0000005,
    0xc0000006, 0xc0000007, 0xc0000008, 0xc0000009,
    0xc000000a, 0xc000000b, 0xc000000c, 0xc000000d,
    0xc000000e, 0xc000000f, 0xc0000010, 0xc0000011,
    0xc0000012, 0xc0000013, 0xc0000014, 0xc0000015,
    0xc0000016, 0xc0000017, 0xc0000018, 0xc0000019,
    0xc000001a, 0xc000001b, 0xc000001c, 0xc000001d,
    0xc000001e, 0xc000001f, 0xb3000000, 0xc3000001,
    0xc3000002, 0xc3000003, 0xc3000004, 0xc3000005,
    0xc3000006, 0xc3000007, 0xc3000008, 0xc3000009,
    0xc300000a, 0xc300000b, 0xc300000c, 0xd300000d,
    0xc300000e, 0xc300000f, 0xbb0c0000, 0xc30c0001,
    0xc30c0002, 0xc30c0003, 0xdb0c0004
};

wint_t btowc(int) {
    return (PROFAN_FNI, 0);
}

int fwprintf(FILE *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int fwscanf(FILE *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

wint_t fgetwc(FILE *) {
    return (PROFAN_FNI, 0);
}

wchar_t *fgetws(wchar_t *, int, FILE *) {
    return (PROFAN_FNI, NULL);
}

wint_t fputwc(wchar_t, FILE *) {
    return (PROFAN_FNI, 0);
}

int fputws(const wchar_t *, FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t getwc(FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t getwchar(void) {
    return (PROFAN_FNI, 0);
}

int mbsinit(const mbstate_t *) {
    return (PROFAN_FNI, 0);
}

size_t mbrlen(const char *, size_t, mbstate_t *) {
    return (PROFAN_FNI, 0);
}

#define OOB(c,b) (((((b) >> 3) - 0x10) | (((b) >> 3) + ((int32_t)(c) >> 26))) & ~7)
#define SA 0xc2u
#define SB 0xf4u

int mbtowc(wchar_t *restrict wc, const char *restrict src, size_t n) {
    const unsigned char *s = (const void *)src;
    wchar_t dummy;
    unsigned c;

    if (!s)
        return 0;
    if (!n)
        goto ilseq;
    if (!wc)
        wc = &dummy;

    if (*s < 0x80)
        return !!(*wc = *s);

    if (MB_CUR_MAX==1) {
        *wc = (0xdfff & (signed char)*s);
        return 1;
    }

    if (*s - SA > SB - SA)
        goto ilseq;

    c = bittab[*s++ - SA];

    if (n < 4 && (( c << (6 * n - 6)) & (1U << 31)))
        goto ilseq;
    if (OOB(c,*s))
        goto ilseq;

    c = (c << 6) | (*s++ - 0x80);

    if (!(c & (1U<<31))) {
        *wc = c;
        return 2;
    }

    if (*s - 0x80u >= 0x40)
        goto ilseq;

    c = (c << 6) | (*s++ - 0x80);
    if (!(c & (1U << 31))) {
        *wc = c;
        return 3;
    }

    if (*s - 0x80u >= 0x40)
        goto ilseq;

    *wc = (c << 6) | (*s++ - 0x80);
    return 4;

ilseq:
    errno = EILSEQ;
    return -1;
}

size_t mbrtowc(wchar_t *restrict wc, const char *restrict src, size_t n, mbstate_t *restrict st) {
    static unsigned internal_state;
    const unsigned char *s = (const void *) src;
    const size_t N = n;
    wchar_t dummy;
    unsigned c;

    if (!st)
        st = (void *) &internal_state;
    c = *(unsigned *) st;

    if (!s) {
        if (c)
            goto ilseq;
        return 0;
    } else if (!wc) {
        wc = &dummy;
    }

    if (!n)
        return -2;

    if (!c) {
        if (*s < 0x80)
            return !!(*wc = *s);
        if (MB_CUR_MAX == 1) {
            *wc = (0xdfff & (signed char)(*s));
            return 1;
        }
        if (*s-SA > SB-SA)
            goto ilseq;
        c = bittab[*s++-SA];
        n--;
    }

    if (n) {
        if (OOB(c,*s))
            goto ilseq;
        loop:
        c = (c << 6) | (*s++ - 0x80);
        n--;
        if (!(c & (1U << 31))) {
            *(unsigned *) st = 0;
            *wc = c;
            return N - n;
        }
        if (n) {
            if (*s-0x80u >= 0x40)
                goto ilseq;
            goto loop;
        }
    }

    *(unsigned *) st = c;
    return -2;

    ilseq:
    *(unsigned *) st = 0;
    errno = EILSEQ;

    return -1;
}

size_t mbsrtowcs(wchar_t *restrict ws, const char **restrict src, size_t wn, mbstate_t *restrict st) {
    const unsigned char *s = (const void *) *src;
    size_t wn0 = wn;
    unsigned c = 0;

    if (st && (c = *(unsigned *)st)) {
        if (ws) {
            *(unsigned *) st = 0;
            goto resume;
        } else {
            goto resume0;
        }
    }

    if (!ws) while (1) {
        if (*s - 1u < 0x7f) {
            s++;
            wn--;
            continue;
        }

        if (*s - SA > SB - SA)
            break;
        c = bittab[*s++ - SA];
resume0:
        if (OOB(c,*s)) {
            s--;
            break;
        }
        s++;
        if (c & (1U << 25)) {
            if (*s - 0x80u >= 0x40) {
                s-=2;
                break;
            }
            s++;

            if (c&(1U<<19)) {
                if (*s - 0x80u >= 0x40) {
                    s -= 3;
                    break;
                }
                s++;
            }
        }
        wn--;
        c = 0;
    } else while (1) {
        if (!wn) {
            *src = (const void *) s;
            return wn0;
        }

        if (*s - 1u < 0x7f) {
            *ws++ = *s++;
            wn--;
            continue;
        }

        if (*s - SA > SB-SA)
            break;
        c = bittab[*s++-SA];
resume:
        if (OOB(c,*s)) {
            s--;
            break;
        }
        c = (c << 6) | (*s++ - 0x80);
        if (c & (1U << 31)) {
            if (*s - 0x80u >= 0x40) {
                s -= 2;
                break;
            }
            c = (c << 6) | (*s++ - 0x80);
            if (c & (1U << 31)) {
                if (*s - 0x80u >= 0x40) {
                    s-=3;
                    break;
                }
                c = ( c<< 6) | (*s++ - 0x80);
            }
        }
        *ws++ = c;
        wn--;
        c = 0;
    }

    if (!c && !*s) {
        if (ws) {
            *ws = 0;
            *src = 0;
        }
        return wn0 - wn;
    }

    if (ws)
        *src = (const void *)s;

    errno = EILSEQ;
    return -1;
}

wint_t putwc(wchar_t, FILE *) {
    return (PROFAN_FNI, 0);
}

wint_t putwchar(wchar_t) {
    return (PROFAN_FNI, 0);
}

int swprintf(wchar_t *, size_t, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int swscanf(const wchar_t *, const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

wint_t ungetwc(wint_t, FILE *) {
    return (PROFAN_FNI, 0);
}

int vfwprintf(FILE *, const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

int vwprintf(const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

int vswprintf(wchar_t *, size_t, const wchar_t *, va_list) {
    return (PROFAN_FNI, 0);
}

size_t wcrtomb(char *restrict s, wchar_t wc, mbstate_t *restrict st) {
    if (!s)
        return 1;

    if ((unsigned) wc < 0x80) {
        *s = wc;
        return 1;
    } else if ((unsigned) wc < 0x800) {
        *s++ = 0xc0 | (wc >> 6);
        *s = 0x80 | (wc & 0x3f);
        return 2;
    } else if ((unsigned) wc < 0xd800 || (unsigned) wc - 0xe000 < 0x2000) {
        *s++ = 0xe0 | (wc >> 12);
        *s++ = 0x80 | ((wc >> 6) & 0x3f);
        *s = 0x80 | (wc & 0x3f);
        return 3;
    } else if ((unsigned) wc - 0x10000 < 0x100000) {
        *s++ = 0xf0 | (wc >> 18);
        *s++ = 0x80 | ((wc >> 12) & 0x3f);
        *s++ = 0x80 | ((wc >> 6) & 0x3f);
        *s = 0x80 | (wc & 0x3f);
        return 4;
    }
    errno = EILSEQ;
    return -1;
}

wchar_t *wcscat(wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcschr(const wchar_t *s, wchar_t c) {
    if (!c)
        return (wchar_t *) s + wcslen(s);

    for (; *s && *s != c; s++);

    return *s ? (wchar_t *)s : 0;
}

int wcscmp(const wchar_t *l, const wchar_t *r) {
    for (; *l == *r && *l && *r; l++, r++);

    return *l < *r ? -1 : *l > *r;
}

int wcscoll(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcscpy(wchar_t *restrict d, const wchar_t *restrict s) {
    wchar_t *a = d;

    while ((*d++ = *s++));

    return a;
}

size_t wcscspn(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

size_t wcsftime(wchar_t *, size_t, const wchar_t *, const struct tm *) {
    return (PROFAN_FNI, 0);
}

size_t wcslen(const wchar_t *s) {
    const wchar_t *a;

    for (a = s; *s; s++);

    return s - a;
}

size_t wcsnlen(const wchar_t *s, size_t n) {
    const wchar_t *z = wmemchr(s, 0, n);

    if (z)
        n = z - s;

    return n;
}

wchar_t *wcsncat(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

int wcsncmp(const wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcsncpy(wchar_t *restrict d, const wchar_t *restrict s, size_t n) {
    wchar_t *a = d;

    while (n && *s) {
        *d++ = *s++;
        n--;
    }

    wmemset(d, 0, n);
    return a;
}

wchar_t *wcspbrk(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcsrchr(const wchar_t *s, wchar_t c) {
    const wchar_t *p;

    for (p = s + wcslen(s); p >= s && *p != c; p--);

    return p >= s ? (wchar_t *) p : 0;
}

size_t wcsrtombs(char *restrict s, const wchar_t **restrict ws, size_t n, mbstate_t *restrict st) {
    const wchar_t *ws2;
    char buf[4];
    size_t N = n, l;

    if (!s) {
        for (n = 0, ws2 = *ws; *ws2; ws2++) {
            if (*ws2 >= 0x80) {
                l = wcrtomb(buf, *ws2, 0);
                if (!(l + 1))
                    return -1;
                n += l;
            } else {
                n++;
            }
        }
        return n;
    }

    while (n>=4) {
        if (**ws - 1u >= 0x7fu) {
            if (!**ws) {
                *s = 0;
                *ws = 0;
                return N-n;
            }
            l = wcrtomb(s, **ws, 0);
            if (!(l + 1))
                return -1;
            s += l;
            n -= l;
        } else {
            *s++ = **ws;
            n--;
        }
        (*ws)++;
    }

    while (n) {
        if (**ws - 1u >= 0x7fu) {
            if (!**ws) {
                *s = 0;
                *ws = 0;
                return N - n;
            }
            l = wcrtomb(buf, **ws, 0);
            if (!(l + 1))
                return -1;
            if (l > n)
                return N - n;
            wcrtomb(s, **ws, 0);
            s += l;
            n -= l;
        } else {
            *s++ = **ws;
            n--;
        }
        (*ws)++;
    }

    return N;
}

size_t wcsspn(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, 0);
}

wchar_t *wcsstr(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcstok(wchar_t *, const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wcswcs(const wchar_t *, const wchar_t *) {
    return (PROFAN_FNI, NULL);
}

int wcswidth(const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

size_t wcsxfrm(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

int wctob(wint_t) {
    return (PROFAN_FNI, 0);
}

int wcwidth(wchar_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wmemchr(const wchar_t *s, wchar_t c, size_t n) {
    for (; n && *s != c; n--, s++);

    return n ? (wchar_t *) s : 0;
}

int wmemcmp(const wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, 0);
}

wchar_t *wmemcpy(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wmemmove(wchar_t *, const wchar_t *, size_t) {
    return (PROFAN_FNI, NULL);
}

wchar_t *wmemset(wchar_t *d, wchar_t c, size_t n) {
    wchar_t *ret = d;

    while (n--)
        *d++ = c;

    return ret;
}

int wprintf(const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

int wscanf(const wchar_t *, ...) {
    return (PROFAN_FNI, 0);
}

double wcstod(const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, 0);
}

float wcstof(const wchar_t *, wchar_t **) {
    return (PROFAN_FNI, 0);
}

long int wcstol(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

long long int wcstoll(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

unsigned long int wcstoul(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}

unsigned long long int wcstoull(const wchar_t *, wchar_t **, int) {
    return (PROFAN_FNI, 0);
}
