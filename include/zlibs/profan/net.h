/*****************************************************************************\
|   === net.h : 2026 ===                                                      |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef NET_H
#define NET_H

#include <stdint.h>

typedef struct eth_info_t {
    uint32_t net_mask;
    uint32_t router_ip;
    uint8_t router_mac[8]; //ignores last 2 bytes

    uint32_t ip;
    uint8_t mac[8]; //ignores last 2 bytes
} eth_info_t;

#endif
