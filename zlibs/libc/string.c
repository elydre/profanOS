/*****************************************************************************\
|   === string.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of string functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <ctype.h>

#define BITOP(a,b,op) \
 ((a)[(size_t)(b)/(8*sizeof *(a))] op (size_t)1<<((size_t)(b)%(8*sizeof *(a))))

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

int ffs(register int mask) {
    register int bit;

    if (mask == 0)
        return 0;

    for (bit = 1; !(mask & 1); bit++)
        mask >>= 1;

    return bit;
}

int ffsll(long long int i) {
    unsigned long long int x = i & -i;

    if (x <= 0xffffffff)
        return ffs (i);
    return 32 + ffs (i >> 32);
}

void *memccpy(void *restrict s1, const void *restrict s2, int c, size_t n) {
    register char *r1 = s1;
    register const char *r2 = s2;

    while (n-- && (((unsigned char)(*r1++ = *r2++)) != ((unsigned char) c)));

    return (n == (size_t) -1) ? NULL : r1;
}

void *memchr(const void *bigptr, int ch, size_t length) {
    const char *big = (const char *) bigptr;
    size_t n;

    for (n = 0; n < length; n++)
        if (big[n] == ch)
            return (void *) &big[n];

    return NULL;
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

    r = ((unsigned char *) s) + ((size_t) n);

    while (n) {
        if (*--r == ((unsigned char) c))
            return (void *) r;
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
    profan_nimpl("psignal");
}

void *rawmemchr(const void *s, int c) {
    register const unsigned char *r = s;

    while (*r != ((unsigned char) c))
        ++r;

    return (void *) r;
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
        if ((*s = *s2) != 0) // Need to fill tail with 0
            s2++;
        ++s;
        --n;
    }
    return s1 + (s2 - p);
}

int strcasecmp(const char *s1, const char *s2) {
    int result = strncasecmp(s1, s2, -1);
    return result;
}

char *strcasestr(const char *s, const char *find) {
    size_t len;
    char c, sc;

    if ((c = *find++) != 0) {
        c = tolower((unsigned char) c);
        len = strlen(find);
        do {
            do {
                if ((sc = *s++) == 0)
                    return (NULL);
            } while ((char) tolower((unsigned char) sc) != c);
        } while (strncasecmp(s, find, len) != 0);
        s--;
    }
    return (char *) s;
}

char *strcat(char *restrict s1, register const char *restrict s2) {
    size_t i,j;
    for (i = 0; s1[i] != '\0'; i++);
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

char *strchrnul(const char *s, int c_in) {
    const unsigned char *char_ptr;
    const unsigned long int *longword_ptr;
    unsigned long int longword, magic_bits, charmask;
    unsigned char c;

    c = (unsigned char) c_in;

    for (char_ptr = (const unsigned char *) s;
            ((unsigned long int) char_ptr & (sizeof (longword) - 1)) != 0; ++char_ptr)
        if (*char_ptr == c || *char_ptr == '\0')
            return (void *) char_ptr;

    longword_ptr = (unsigned long int *) char_ptr;

    magic_bits = -1;
    magic_bits = magic_bits / 0xff * 0xfe << 1 >> 1 | 1;

    charmask = c | (c << 8);
    charmask |= charmask << 16;
    if (sizeof (longword) > 4)
        charmask |= (charmask << 16) << 16;
    if (sizeof (longword) > 8)
        abort();

    for (;;) {
      longword = *longword_ptr++;

        if ((((longword + magic_bits) ^ ~longword) & ~magic_bits) != 0 ||
          ((((longword ^ charmask) + magic_bits) ^ ~(longword ^ charmask))
            & ~magic_bits) != 0) {
        const unsigned char *cp = (const unsigned char *) (longword_ptr - 1);

        if (*cp == c || *cp == '\0')
            return (char *) cp;
        if (*++cp == c || *cp == '\0')
            return (char *) cp;
        if (*++cp == c || *cp == '\0')
            return (char *) cp;
        if (*++cp == c || *cp == '\0')
            return (char *) cp;
        if (sizeof(longword) > 4) {
            if (*++cp == c || *cp == '\0')
                return (char *) cp;
            if (*++cp == c || *cp == '\0')
                return (char *) cp;
            if (*++cp == c || *cp == '\0')
                return (char *) cp;
            if (*++cp == c || *cp == '\0')
                return (char *) cp;
            }
        }
    }

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

size_t strcspn(const char *s1, register const char *s2) {
    register const char *p, *spanp;
    register char c, sc;

    for (p = s1;;) {
        c = *p++;
        spanp = s2;
        do {
            if ((sc = *spanp++) == c)
                return (p - 1 - s1);
        } while (sc != 0);
    }
    // NOTREACHED
    return 0;
}

size_t strlen(const char *s);
char *strdup(register const char *s) {
    size_t l = strlen(s);
    char *d = malloc(l+1);
    if (!d) return NULL;
    return memcpy(d, s, l+1);
}

// strerror is defined in errno.c

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
    unsigned char *ucs1 = (unsigned char *) s1;
    unsigned char *ucs2 = (unsigned char *) s2;
    int d = 0;
    for ( ; n != 0; n--) {
        int c1 = tolower(*ucs1);
        int c2 = tolower(*ucs2);
        if (((d = c1 - c2) != 0) || (c2 == '\0')) break;
        ucs1++;
        ucs2++;
    }
    return d;
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

char *strndup(const char *str, size_t n) {
    size_t len;
    char *copy;

    for (len = 0; len < n && str[len]; len++);

    if ((copy = malloc(len + 1)) == NULL)
        return NULL;

    memcpy(copy, str, len);
    copy[len] = '\0';
    return copy;
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
            if (*p == *s)
                return (char *) s;
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

    return (char *) p;
}

char *strsep(register char **stringp, register const char *delim) {
    register const char *spanp;
    register int c, sc;
    register char *s;
    char *tok;

    if ((s = *stringp) == NULL)
        return (NULL);

    for (tok = s;;) {
        c = *s++;
        spanp = delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                *stringp = s;
                return (tok);
            }
        } while (sc != 0);
    }
    // NOTREACHED
}

char *strsignal(int signum) {
    profan_nimpl("strsignal");
    return NULL;
}

size_t strspn(const char *s, const char *c) {
    const char *a = s;
    size_t byteset[32 / sizeof(size_t)] = { 0 };

    if (!c[0])
        return 0;

    if (!c[1]) {
        for (; *s == *c; s++);
        return s-a;
    }

    for (; *c && BITOP(byteset, *(unsigned char *)c, |=); c++);
    for (; *s && BITOP(byteset, *(unsigned char *)s, &); s++);
    return s-a;
}

char *strstr(register const char *string, const char *substring) {
    register const char *a, *b;

    /* First scan quickly through the two strings looking for a
     * single-character match.  When it's found, then compare the
     * rest of the substring.
     */

    b = substring;
    if (*b == 0) {
        return (char *) string;
    }
    for ( ; *string != 0; string += 1) {
        if (*string != *b) {
            continue;
        }
        a = string;
        while (1) {
            if (*b == 0)
                return (char *) string;
            if (*a++ != *b++)
                break;
        }
        b = substring;
    }
    return NULL;
}

char *strtok(register char *s, register const char *delim) {
    register char *spanp;
    register int c, sc;
    static char *last;
    char *tok;

    if (s == NULL && (s = last) == NULL)
        return (NULL);

cont:
    c = *s++;
    for (spanp = (char *)delim; (sc = *spanp++) != 0;) {
        if (c == sc)
            goto cont;
    }

    if (c == 0) {       // no non-delimiter characters
        last = NULL;
        return (NULL);
    }
    tok = s - 1;

    for (;;) {
        c = *s++;
        spanp = (char *)delim;
        do {
            if ((sc = *spanp++) == c) {
                if (c == 0)
                    s = NULL;
                else
                    s[-1] = 0;
                last = s;
                return (tok);
            }
        } while (sc != 0);
    }
    // NOTREACHED
}

char *strtok_r(char *s, const char *delim, char **save_ptr) {
    char *end;

    if (s == NULL)
        s = *save_ptr;

    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }

    // Scan leading delimiters
    s += strspn(s, delim);
    if (*s == '\0') {
        *save_ptr = s;
        return NULL;
    }

    // Find the end of the token
    end = s + strcspn(s, delim);
    if (*end == '\0') {
        *save_ptr = end;
        return s;
    }

    *end = '\0';
    *save_ptr = end + 1;
    return s;
}

int strverscmp(const char *s1, const char *s2) {
    profan_nimpl("strverscmp");
    return 0;
}
