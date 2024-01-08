#include <syscall.h>
#include <stdlib.h>
#include <stdio.h>
#include <type.h>

#ifndef TOLOWER
#define TOLOWER(c) ((c) >= 'A' && (c) <= 'Z' ? (c) + 'a' - 'A' : (c))
#endif

#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

void init_func();

int main(void) {
    init_func();
    return 0;
}

void init_func(void) {; }

char *basename(const char *path) {
    register const char *s;
    register const char *p;

    p = s = path;

    while (*s) {
        if (*s++ == '/') {
            p = s;
        }
    }

    return (char *) p;
}

void bcopy(const void *s2, void *s1, size_t n) {
    register char *s;
    register const char *p;

    s = s1;
    p = s2;
    if (p >= s) {
        while (n) {
            *s++ = *p++;
            --n;
        }
    } else {
        while (n) {
            --n;
            s[n] = p[n];
        }
    }
}

void bzero(void *s, size_t n) {
    register unsigned char *p = s;

    while (n) {
        *p++ = 0;
        --n;
    }
}


char *dirname(char *path) {
    static const char null_or_empty_or_noslash[] = ".";
    register char *s;
    register char *last;
    char *first;

    last = s = path;

    if (s != NULL) {

    LOOP:
        while (*s && (*s != '/')) ++s;
        first = s;
        while (*s == '/') ++s;
        if (*s) {
            last = first;
            goto LOOP;
        }

        if (last == path) {
            if (*last != '/') {
                goto DOT;
            }
            if ((*++last == '/') && (last[1] == 0)) {
                ++last;
            }
        }
        *last = 0;
        return path;
    }
 DOT:
    return (char *) null_or_empty_or_noslash;
}

int ffs(int i) {
    puts("ffs not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int ffsll(long long int i) {
    puts("ffsll not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n) {
    register char *r1 = s1;
    register const char *r2 = s2;

    while (n-- && (((unsigned char)(*r1++ = *r2++)) != ((unsigned char) c)));

    return (n == (size_t) -1) ? NULL : r1;
}

void *memchr(const void *s, int c, size_t n) {
    puts("memchr not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

int memcmp(const void *s1, const void *s2, size_t n) {
    register const uint8_t *r1 = (const uint8_t *) s1;
    register const uint8_t *r2 = (const uint8_t *) s2;

    int r = 0;

    while (n-- && ((r = ((int)(*r1++)) - *r2++) == 0));
    return r;
}

void *memcpy(void *restrict s1, const void *restrict s2, size_t n) {
    register char *r1 = s1;
    register const char *r2 = s2;

    while (n--) {
        *r1++ = *r2++;
    }

    return s1;
}

void *memmem(const void *haystack, size_t haystacklen,
             const void *needle, size_t needlelen) {
    register const char *ph;
    register const char *pn;
    const char *plast;
    size_t n;

    if (needlelen == 0) {
        return (void *) haystack;
    }

    if (haystacklen >= needlelen) {
        ph = (const char *) haystack;
        pn = (const char *) needle;
        plast = ph + (haystacklen - needlelen);

        do {
            n = 0;
            while (ph[n] == pn[n]) {
                if (++n == needlelen) {
                    return (void *) ph;
                }
            }
        } while (++ph <= plast);
    }

    return NULL;
}

void *memmove(void *s1, const void *s2, size_t n) {
    register char *s = (char *) s1;
    register const char *p = (const char *) s2;

    if (p >= s) {
        while (n) {
            *s++ = *p++;
            --n;
        }
    } else {
        while (n) {
            --n;
            s[n] = p[n];
        }
    }

    return s1;
}

void *mempcpy(void *restrict s1, const void *restrict s2, size_t n) {
   register char *r1 = s1;
   register const char *r2 = s2;

   while (n) {
      *r1++ = *r2++;
      --n;
   }

   return r1;
}

void *memrchr(const void *s, int c, size_t n) {
    register const unsigned char *r;

    r = ((unsigned char *)s) + ((size_t) n);

    while (n) {
        if (*--r == ((unsigned char)c)) {
            return (void *) r;    /* silence the warning */
        }
        --n;
    }

    return NULL;
}

void *memset(void *s, int c, size_t n) {
    register uint8_t *p = (uint8_t *) s;

    while (n) {
        *p++ = (uint8_t) c;
        --n;
    }

    return s;
}

void psignal(int signum, register const char *message) {
    puts("psignal not implemented yet, WHY DO YOU USE IT ?");
}

void *rawmemchr(const void *s, int c) {
    register const unsigned char *r = s;

    while (*r != ((unsigned char)c)) ++r;

    return (void *) r;    /* silence the warning */
}

char *stpcpy(register char *restrict s1, const char *restrict s2) {
    while ((*s1++ = *s2++) != 0);

    return s1 - 1;
}

char *stpncpy(register char *restrict s1,
                register const char *restrict s2,
                size_t n) {
    char *s = s1;
    const char *p = s2;

    while (n) {
        if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }
    return s1 + (s2 - p);
}

int strncasecmp(register const char *s1, register const char *s2, size_t n);

int strcasecmp (const char *s1, const char *s2) {
    int result = strncasecmp(s1, s2, -1);
    return result;
}

int strcasecmp_l(register const char *s1, register const char *s2, locale_t loc) {
    puts("strcasecmp_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *strcasestr(const char *s1, const char *s2) {
    puts("strcasestr not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *strcat(char *restrict s1, register const char *restrict s2) {
    size_t i,j;
    for (i = 0; s1[i] != '\0'; i++)
        ;
    for (j = 0; s2[j] != '\0'; j++)
        s1[i+j] = s2[j];
    s1[i+j] = '\0';
    return s1;
}

char *strchr(const char *p, int ch) {
    char c;
    c = ch;
    for (;; ++p) {
        if (*p == c)
            return (char *)p;
        if (*p == '\0')
            return NULL;
    }
}

char *strchrnul(register const char *s, int c) {
    puts("strchrnul not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int strcmp(register const char *s1, register const char *s2) {
    while (*s1 == *s2++) {
        if (*s1++ == 0) {
            return 0;
        }
    }
    return *(unsigned char *) s1 - *(unsigned char *) --s2;
}

char *strcpy(char *restrict s1, const char *restrict s2) {
    int i;
    for (i = 0; s2[i] != '\0'; ++i) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
    return s1;
}

size_t strcspn(const char *s1, const char *s2) {
    puts("strcspn not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

size_t strlen(const char *s);
char *strdup(register const char *s) {
    size_t l = strlen(s);
    char *d = malloc(l+1);
    if (!d) return NULL;
    return memcpy(d, s, l+1);
}

char *strerror(int errnum) {
    puts("strerror not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

size_t strlcat(register char *restrict dst,
               register const char *restrict src,
               size_t n) {
    size_t len;
    char dummy[1];

    len = 0;

    while (1) {
        if (len >= n) {
            dst = dummy;
            break;
        }
        if (!*dst) {
            break;
        }
        ++dst;
        ++len;
    }

    while ((*dst = *src) != 0) {
        if (++len < n) {
            ++dst;
        }
        ++src;
    }

    return len;
}

size_t strlcpy(register char *restrict dst,
                  register const char *restrict src,
                  size_t n) {
    const char *src0 = src;
    char dummy[1];

    if (!n) {
        dst = dummy;
    } else {
        --n;
    }

    while ((*dst = *src) != 0) {
        if (n) {
            --n;
            ++dst;
        }
        ++src;
    }

    return src - src0;
}

size_t strlen(const char *s) {
    register const char *p;

    for (p=s ; *p ; p++);

    return p - s;
}

int strncasecmp(register const char *s1, register const char *s2, size_t n) {
    int is = strcmp(s1, "PNAMES");
    unsigned char *ucs1 = (unsigned char *) s1;
    unsigned char *ucs2 = (unsigned char *) s2;
    int d = 0;
    for ( ; n != 0; n--) {
        int c1 = TOLOWER(*ucs1);
        int c2 = TOLOWER(*ucs2);
        if (((d = c1 - c2) != 0) || (c2 == '\0')) break;
        ucs1++;
        ucs2++;
    }
    return d;
}

int strncasecmp_l(register const char *s1, register const char *s2, size_t n, locale_t loc) {
    puts("strncasecmp_l not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}

char *strncat(char *restrict s1, register const char *restrict s2,
                size_t n) {
    register char *s = s1;

    while (*s++);
    --s;
    while (n && ((*s = *s2++) != 0)) {
        --n;
        ++s;
    }
    *s = 0;

    return s1;
}

int strncmp(register const char *s1, register const char *s2, size_t n) {
    if (n == 0) return 0;
    do {
        if (*s1 != *s2++)
            return *(unsigned char *) s1 - *(unsigned char *) --s2;
        if (*s1++ == 0)
            break;
    } while (--n != 0);
    return 0;
}


char *strncpy(char *restrict s1, register const char *restrict s2,
               size_t n) {
    register char *s = s1;

    while (n) {
        if ((*s = *s2) != 0) s2++; /* Need to fill tail with 0s. */
        ++s;
        --n;
    }

    return s1;
}

char *strndup(register const char *s1, size_t n) {
    puts("strndup not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

size_t strnlen(const char *s, size_t max) {
    register const char *p = s;

    while (max && *p) {
        ++p;
        --max;
    }

    return p - s;
}

char *strpbrk(const char *s1, const char *s2) {
    register const char *s;
    register const char *p;

    for (s=s1 ; *s ; s++) {
        for (p=s2 ; *p ; p++) {
            if (*p == *s) return (char *) s; /* silence the warning */
        }
    }
    return NULL;
}

char *strrchr(register const  char *s, int c) {
    register const char *p;

    p = NULL;
    do {
        if (*s == (char) c) {
            p = s;
        }
    } while (*s++);

    return (char *) p;            /* silence the warning */
}

char *strsep(char **restrict s1, const char *restrict s2) {
    puts("strsep not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

char *strsignal(int signum) {
    puts("strsignal not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

size_t strspn(const char *s, const char *c) {
    const char *a = s;
    size_t byteset[32 / sizeof(size_t)] = { 0 };

    if (!c[0]) return 0;
    if (!c[1]) {
        for (; *s == *c; s++);
        return s-a;
    }

    for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
    for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);
    return s-a;
}

char *strstr(register char *string, char *substring) {
    register char *a, *b;

    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */

    b = substring;
    if (*b == 0) {
        return string;
    }
    for ( ; *string != 0; string += 1) {
        if (*string != *b) {
            continue;
        }
        a = string;
        while (1) {
            if (*b == 0) {
                return string;
            }
            if (*a++ != *b++) {
                break;
            }
        }
        b = substring;
    }
    return NULL;
}

char *strtok(char *restrict s1, const char *restrict s2) {
    puts("strtok not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

char *strtok_r(char *restrict s1, const char *restrict s2,
                 char **restrict next_start) {
    puts("strtok_r not implemented yet, WHY DO YOU USE IT ?");
    return NULL;
}

int strverscmp(const char *s1, const char *s2) {
    puts("strverscmp not implemented yet, WHY DO YOU USE IT ?");
    return 0;
}
