/*****************************************************************************\
|   === get_rw.c : 2026 ===                                                   |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "udp.h"

int socket_get_rw(int id) {
    socket_t *sock = socket_find_id(id);
    if (!sock)
        return -1;

    switch (sock->type) {
        case SOCKET_UDP:
            return socket_udp_get_rw(sock);
        default:
            return -1;
    }
}
