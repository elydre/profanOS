/*****************************************************************************\
|   === bind.c : 2026 ===                                                     |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/socket.h>
#include <errno.h>
#include "udp.h"

int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
    socket_t *sock = socket_find_fd(sockfd);
    if (!sock)
        return -ENOTSOCK;
    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->bind)
        return -EINVAL;
    return prot->bind(sock, addr, addrlen);
}
