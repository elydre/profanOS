/*****************************************************************************\
|   === inet.c : 2025 ===                                                     |
|                                                                             |
|    Implementation of arpa/inet functions from libC               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <arpa/inet.h>
#include <inttypes.h>
#include <stdlib.h>
#include <ctype.h>

uint32_t htonl(uint32_t hostlong) {
    return ((hostlong & 0xFF000000) >> 24) | ((hostlong & 0x00FF0000) >> 8) |
           ((hostlong & 0x0000FF00) << 8) | ((hostlong & 0x000000FF) << 24);
}

uint16_t htons(uint16_t hostshort) {
    return (hostshort << 8) | (hostshort >> 8);
}

uint32_t ntohl(uint32_t netlong) {
    return htonl(netlong);
}

uint16_t ntohs(uint16_t netshort) {
    return htons(netshort);
}

int inet_aton(const char *str, struct in_addr *dest) {
    unsigned char *d = (void *)dest;
    unsigned long a[4] = { 0 };
    const char *s = str;
    char *z;
    int i;

    for (i = 0; i < 4; i++) {
        a[i] = strtoul(s, &z, 0);
        if (z == s || (*z && *z != '.') || !isdigit(*s))
            return 0;
        if (*z == '\0')
            break;
        s = z + 1;
    }

    if (i == 4)
        return 0;

    switch (i) {
        case 0:
            a[1] = a[0] & 0xffffff;
            a[0] >>= 24;
            // fall through
        case 1:
            a[2] = a[1] & 0xffff;
            a[1] >>= 16;
            // fall through
        case 2:
            a[3] = a[2] & 0xff;
            a[2] >>= 8;
    }

    for (i = 0; i < 4; i++) {
        if (a[i] > 255)
            return 0;
        d[i] = a[i];
    }

    return 1;
}
