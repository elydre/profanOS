/*****************************************************************************\
|   === ip.c : 2025 ===                                                       |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <net/ethernet.h>
#include <net/ip.h>

uint16_t ip_checksum(void *vdata, uint32_t length) {
    uint16_t *data = vdata;
    uint32_t sum = 0;

    for (; length > 1; length -= 2) {
        sum += *data++;
    }

    if (length == 1) {
        sum += *(uint8_t *)data;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}
