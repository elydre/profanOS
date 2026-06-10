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

#include <modules/socket.h>
#include <fcntl.h> // For O_NONBLOCK
#include <errno.h>

ssize_t socket_recvfrom(recvfrom_arg_t *args) {
    int sockfd = args->sockfd;
    void *buf = args->buf;
    size_t len = args->len;
    int flags = args->flags;
    struct sockaddr *src_addr = args->src_addr;
    socklen_t *addrlen = args->addrlen;
    fd_data_t *data = fm_fd_to_data(sockfd);
    if (!data || data->type != TYPE_SOCK)
        return -ENOTSOCK;
    socket_t *sock = socket_find_id(data->sock_id);
    if (!sock)
        return -ENOTSOCK;

    protocol_t *prot = socket_find_protocol(sock->type);
    if (!prot || !prot->recvfrom)
        return -EINVAL;

    if (data->flags & O_NONBLOCK)
        flags |= MSG_DONTWAIT;

     return prot->recvfrom(sock, buf, len, flags, src_addr, addrlen);
}
