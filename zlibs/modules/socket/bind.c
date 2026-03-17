/*****************************************************************************\
|   === bind.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
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
    switch (sock->type) {
        case SOCKET_UDP:
            return socket_udp_bind(sock, addr, addrlen);
        default:
            return -EINVAL;
    }
}
