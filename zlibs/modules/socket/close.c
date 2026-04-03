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

void socket_close(socket_t *sock) {
    sock->ref_count--;
}

int socket_close_id(int id) {
    socket_t *sock = socket_find_id(id);
    if (!sock)
        return -EINVAL;
    socket_close(sock);
    return 0;
}
