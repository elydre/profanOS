/*****************************************************************************\
|   === recvfrom.c : 2026 ===                                                 |
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

ssize_t socket_recvfrom(recvfrom_arg_t *args) {
    int sockfd = args->sockfd;
    void *buf = args->buf;
    size_t len = args->len;
    int flags = args->flags;
    struct sockaddr *src_addr = args->src_addr;
    socklen_t *addrlen = args->addrlen;
    socket_t *sock = socket_find_fd(sockfd);
    if (!sock)
        return -ENOTSOCK;

    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->recvfrom)
        return -EINVAL;

     return prot->recvfrom(sock, buf, len, flags, src_addr, addrlen);
}
