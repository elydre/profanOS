/*****************************************************************************\
|   === get_rw.c : 2026 ===                                                   |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
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
    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->get_rw)
        return -1;
    return prot->get_rw(sock);
}
