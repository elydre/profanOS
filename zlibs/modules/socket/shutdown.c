/*****************************************************************************\
|   === shutdown.c : 2026 ===                                                 |
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

ssize_t socket_shutdown(int sockfd, int how) {
    socket_t *sock = socket_find_fd(sockfd);
    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->shutdown)
        return -EINVAL;
    if (how != SHUT_RD && how != SHUT_WR && how != SHUT_RDWR)
        return -EINVAL;
    return prot->shutdown(sock, how);
}
