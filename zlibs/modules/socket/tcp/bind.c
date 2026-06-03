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

#include <minilib.h>
#include <errno.h>
#include "utils.h"

#include "tcp.h"

int socket_tcp_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
    tcp_t *data = sock->data;
    if (addrlen != sizeof(struct sockaddr_in))
        return -EINVAL;
    if (TCP_GET_INFO(data, TCP_BIND_MASK))
        return -EINVAL;
    const struct sockaddr_in *addr2 = (void *)addr;
    if (addr2->sin_family != AF_INET)
        return -EINVAL;
    uint16_t port = addr2->sin_port;
    if (port && !tcp_is_port_free(port))
        return -EADDRINUSE;
    if (port == 0)
        port = tcp_get_free_port();
    if (port == 0)
        return -ENOMEM;
    tcp_lock_port(htons(port));
    data->local_ip = addr2->sin_addr.s_addr;
    data->local_port = port;
    TCP_SET_INFO(data, TCP_BIND_MASK);
    return 0;
}
