/*****************************************************************************\
|   === close.c : 2026 ===                                                    |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <errno.h>
#include "udp.h"

int socket_close(socket_t *sock) {
    sock->ref_count--;
    if (sock->ref_count)
        return 0;
    sock->parent_pid = -1;
    return 0;
}

int socket_close_id(int id) {
    socket_t *sock = socket_find_id(id);
    if (!sock)
        return -EINVAL;
    return socket_close(sock);
}
