/*****************************************************************************\
|   === protocols.c : 2026 ===                                                |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>
#include "udp.h"
#include "tcp.h"

protocol_t socket_protocols[] = {
    {
        SOCKET_UDP,
        socket_udp_init,
        socket_udp_bind,
        socket_udp_connect,
        socket_udp_sendto,
        socket_udp_recvfrom,
        socket_udp_get_rw,
    },
    {
        SOCKET_TCP,
        socket_tcp_init,
        socket_tcp_bind,
        socket_tcp_connect,
        socket_tcp_sendto,
        socket_tcp_recvfrom,
        socket_tcp_get_rw,
    },
    {0},
};

protocol_t *socket_find_protocol(uint32_t type) {
    for (size_t i = 0; socket_protocols[i].prot; i++) {
        if (socket_protocols[i].prot == type)
            return &socket_protocols[i];
    }
    return NULL;
}
