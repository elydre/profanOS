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

#include "tcp.h"
#include <kernel/process.h>
#include <errno.h>

ssize_t socket_tcp_recvfrom(socket_t *sock, void *buf, size_t len,
                int flags, struct sockaddr *src_addr, socklen_t *addrlen) {

    (void)src_addr;
    (void)addrlen;
    tcp_t *data = sock->data;

    if (data->state == TCP_STATE_CLOSED)
        return -EAGAIN;
    if (data->recv == NULL)
        return 0;

    while (data->recv_len == 0) {
        if (data->state != TCP_STATE_OPEN)
            break;
        process_sleep(process_get_pid(), 5);
    }
    if (data->recv_len == 0 || data->recv == NULL)
        return 0;
    ssize_t to_read = TCP_MIN(data->recv_len, len);
    mem_copy(buf, data->recv, to_read);
    data->recv_len -= to_read;
    mem_copy(data->recv, &data->recv[to_read], data->recv_len);

    if (TCP_GET_INFO(data, TCP_RECV_FIN_MASK) && data->recv_len == 0) {
        free(data->recv);
        data->recv = NULL;
        data->recv_len = 0;
        data->recv_max = 0;
    }

    return to_read;
}
