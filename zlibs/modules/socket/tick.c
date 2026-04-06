/*****************************************************************************\
|   === tick.c : 2026 ===                                                     |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>
#include <kernel/process.h>
#include <system.h>

#include "udp.h"
#include "ip.h"
#include "arp.h"

#define ETHER_IP4 0x0800
#define ETHER_ARP 0x8006

void socket_tick(int len, uint8_t *packet) {
    for (int i = 0; i < sockets_len; i++) {
        switch (sockets[i].type) {
            case SOCKET_UDP:
                socket_udp_tick(&sockets[i]);
                break;
            default:
                sys_warning("%d %d %d\n", AF_INET, SOCK_DGRAM, 0);
                sys_warning("Invalid socket type %d %d %d\n",
                    sockets[i].type & 0xff,
                    (sockets[i].type >> 8) & 0xff,
                    (sockets[i].type >> 16));
                break;
        }
    }

    if (len >= 6 + 6 + 2) {
        uint16_t ether_type = packet[6 + 6] << 8;
        ether_type |= packet[6 + 6 + 1];
        len -= 6 + 6 + 2;
        packet += 6 + 6 + 2;
        if (len) {
            switch (ether_type) {
                case ETHER_IP4:
                    socket_on_recv_ip(len, packet);
                    break;
				case ETHER_ARP:
					socket_on_recv_arp(len, packet);
					break;
                default:
                    break;
            }
        }
    }
    int k = 0;
    for (int i = 0; i < sockets_len; i++) {
        if (!sockets[i].do_remove) {
            socket_t tmp = sockets[k];
            sockets[k] = sockets[i];
            sockets[i] = tmp;

            k++;
        }
    }
    int last_alive = 0;
    while (last_alive < sockets_len && !sockets[last_alive].do_remove)
        last_alive++;
    sockets_len = last_alive;
}
