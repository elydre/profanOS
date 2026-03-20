/*****************************************************************************\
|   === ip.h : 2026 ===                                                       |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef IP_H
#define IP_H

#include <modules/socket.h>

typedef struct ip_header_t {
    uint8_t  v_ihl;
    uint8_t  tos;
    uint16_t tot_len;
    uint16_t id;
    uint16_t flags_n_offset;
    uint8_t  ttl;
    uint8_t  protocol;
    uint16_t checksum;
    uint32_t src_ip;
    uint32_t dest_ip;
    uint8_t *options;
    int options_len;
    uint8_t *data;
    int data_len;
} ip_header_t;

void socket_on_recv_ip(int len, uint8_t *packet);
void socket_on_send_ip(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, uint8_t *data, int data_len);

#endif
