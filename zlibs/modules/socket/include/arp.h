/*****************************************************************************\
|   === arp.h : 2026 ===                                                      |
|                                                                             |
|    Header for the Address Resolution Procotol (ARP)              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef ARP_H
#define ARP_H

#include "utils.h"
#include <modules/socket.h>
#include <modules/eth.h>

typedef struct {
    uint16_t htype;
    uint16_t ptype;
    uint8_t hlen;
    uint8_t plen;
    uint16_t op;
    uint8_t s_haddr[6];
    uint32_t s_paddr;
    uint8_t t_haddr[6];
    uint32_t t_paddr;
} arp_packet_t;

void socket_on_recv_arp(int len, uint8_t *packet);

#endif
