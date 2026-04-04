/*****************************************************************************\
|   === ports.c : 2026 ===                                                    |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "udp.h"
#include "utils.h"

static uint8_t bitmap[0xFFFF / 8] = {0};

int udp_is_port_free(uint16_t port) {
    return !((bitmap[port >> 3] >> (port & 7)) & 1);
}

void udp_lock_port(uint16_t port) {
    bitmap[port >> 3] |= 1 << (port & 7);
}

void udp_free_port(uint16_t port) {
    bitmap[port >> 3] &= ~(1 << (port & 7));
}

uint16_t udp_get_free_port() {
    uint16_t res = CLT_PORT_START;
    while (res <= CLT_PORT_END) {
        if (!udp_is_port_free(res))
            res++;
        return htons(res);
    }
    return 0;
}
