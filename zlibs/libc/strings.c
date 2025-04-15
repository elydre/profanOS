/*****************************************************************************\
|   === strings.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of strings functions from libC                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <strings.h>
#include <string.h>
#include <ctype.h>

int bcmp(const void *s1, const void *s2, size_t n) {
    return memcmp(s1, s2, n);
}

void bcopy(const void *s1, void *s2, size_t n) {
    memmove(s2, s1, n);
}

void bzero(void *s, size_t n) {
    memset(s, 0, n);
}

int ffs(int mask) {
    int bit;

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

char *index(const char *s, int c) {
    return strchr(s, c);
}

char *rindex(const char *s, int c) {
    return strrchr(s, c);
}

int strcasecmp(const char *s1, const char *s2) {
    const unsigned char *l, *r;

    l = (void *) s1;
    r = (void *) s2;

    for (; *l && *r && (*l == *r || tolower(*l) == tolower(*r)); l++, r++);

    return tolower(*l) - tolower(*r);
}

int strncasecmp(const char *s1, const char *s2, size_t n) {
    const unsigned char *l, *r;

    l = (void *) s1;
    r = (void *) s2;

    if (!n--)
        return 0;

    for (; *l && *r && n && (*l == *r || tolower(*l) == tolower(*r));
            l++, r++, n--);

    return tolower(*l) - tolower(*r);
}

char *strcasestr(const char *h, const char *n) {
    size_t l = strlen(n);

    for (; *h; h++) {
        if (strncasecmp(h, n, l) == 0)
            return (char *) h;
    }

    return NULL;
}
