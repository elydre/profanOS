/*****************************************************************************\
|   === connect.c : 2026 ===                                                  |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <minilib.h>

#include "tcp.h"

int socket_tcp_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
    if (addrlen != sizeof(struct sockaddr_in))
        return -EINVAL;

    const struct sockaddr_in *addr2 = (void *) addr;

    if (addr2->sin_family != AF_INET)
        return -EINVAL;

    if (addr2->sin_port == 0)
        return -EINVAL;
    
    tcp_t *data = sock->data;
    if (data->is_connected)
        return -EISCONN;

    if (!data->is_bound) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = info.ip;
        addr.sin_port = 0;

        int err = socket_tcp_bind(sock, (void *)&addr, sizeof(addr));
        if (err)
            return err;
    }

    data->is_connected = 1;
    data->remote_port = addr2->sin_port;
    data->remote_ip = addr2->sin_addr.s_addr;
    data->state = TCP_STATE_SYN_SENT;
    tcp_send_syn(data);
    data->retries = 0;

    return 0;
}
