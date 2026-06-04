/*****************************************************************************\
|   === getsockname.c : 2026 ===                                              |
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

int socket_getname(int sockfd, int local, struct sockaddr *addr, socklen_t *addrlen) {
    socket_t *sock = socket_find_fd(sockfd);
    if (!sock)
        return -EBADF;

    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->getname)
        return -EINVAL;

    return prot->getname(sock, local, addr, addrlen);
}
