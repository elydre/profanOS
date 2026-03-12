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
#include <string.h>
#include <errno.h>
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

static int hexval(unsigned c) {
	if (c - '0' < 10)
        return c - '0';
	c |= 32;
	if (c - 'a' < 6)
        return c-'a'+10;
	return -1;
}

int inet_pton(int af, const char *s, void *a0) {
	int i, j, v, d, brk = -1, need_v4 = 0;
	unsigned char *a = a0;
	uint16_t ip[8];

	if (af == AF_INET) {
		for (i = 0; i < 4; i++) {
			for (v = j = 0; j < 3 && isdigit(s[j]); j++)
				v = 10 * v + s[j] - '0';
			if (j == 0 || (j > 1 && s[0] == '0') || v > 255)
                return 0;
			a[i] = v;
			if (s[j] == 0 && i == 3)
                return 1;
			if (s[j] != '.')
                return 0;
			s += j + 1;
		}
		return 0;
	} else if (af != AF_INET6) {
		errno = EAFNOSUPPORT;
		return -1;
	}

	if (*s == ':' && *++s != ':')
        return 0;

	for (i = 0;; i++) {
		if (s[0] == ':' && brk < 0) {
			brk = i;
			ip[i & 7] = 0;
			if (!*++s)
                break;
			if (i == 7)
                return 0;
			continue;
		}

		for (v = j = 0; j < 4 && (d = hexval(s[j])) >= 0; j++)
			v = 16 * v + d;
		if (j == 0)
            return 0;
    
		ip[i & 7] = v;

		if (!s[j] && (brk >= 0 || i == 7))
            break;
		if (i == 7)
            return 0;
		if (s[j] != ':') {
			if (s[j] != '.' || (i < 6 && brk < 0))
                return 0;
			need_v4 = 1;
			i++;
			ip[i & 7] = 0;
			break;
		}
		s += j + 1;
	}
	
    if (brk >= 0) {
		memmove(ip + brk + 7 - i, ip + brk, 2 * (i + 1 - brk));
		for (j = 0; j < 7 - i; j++)
            ip[brk+j] = 0;
	}
	
    for (j = 0; j < 8; j++) {
		*a++ = ip[j] >> 8;
		*a++ = ip[j];
	}

	return !(need_v4 && inet_pton(AF_INET, (void *) s, a - 4) <= 0);
}
