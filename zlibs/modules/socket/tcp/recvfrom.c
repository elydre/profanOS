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

static ssize_t read_block(socket_t *sock, void *buf, size_t len, int peek) {
    tcp_t *data = sock->data;

    while (data->recv_len == 0) {
        if (data->state != TCP_STATE_OPEN)
            break;
        process_sleep(process_get_pid(), 5);
    }
    if (data->recv_len == 0 || data->recv == NULL)
        return 0;
    ssize_t to_read = TCP_MIN(data->recv_len, len);
    mem_copy(buf, data->recv, to_read);
    if (!peek) {
        data->recv_len -= to_read;
        mem_copy(data->recv, &data->recv[to_read], data->recv_len);
    }

    if (TCP_GET_INFO(data, TCP_RECV_FIN_MASK) && data->recv_len == 0) {
        free(data->recv);
        data->recv = NULL;
        data->recv_len = 0;
        data->recv_max = 0;
    }

    return to_read;
}

static ssize_t read_all(socket_t *sock, void *buf, size_t len) {
    size_t total_read = 0;
    while (total_read < len) {
        ssize_t ret = read_block(sock, (char *)buf + total_read, len - total_read, 0);
        if (ret <= 0)
            return ret < 0 ? ret : (ssize_t) total_read;
        total_read += ret;
    }
    return total_read;
}

static ssize_t read_nonblock(socket_t *sock, void *buf, size_t len, int peek) {
    tcp_t *data = sock->data;

    if (data->recv_len == 0 || data->recv == NULL)
        return -EAGAIN;
    ssize_t to_read = TCP_MIN(data->recv_len, len);
    mem_copy(buf, data->recv, to_read);
    if (!peek) {
        data->recv_len -= to_read;
        mem_copy(data->recv, &data->recv[to_read], data->recv_len);

        if (TCP_GET_INFO(data, TCP_RECV_FIN_MASK) && data->recv_len == 0) {
            free(data->recv);
            data->recv = NULL;
            data->recv_len = 0;
            data->recv_max = 0;
        }
    }
    return to_read;
}


ssize_t socket_tcp_recvfrom(socket_t *sock, void *buf, size_t len,
                int flags, struct sockaddr *src_addr, socklen_t *addrlen) {

    /*
    (DONTWAIT/NONBLOCK, PEAK)
    (WAITALL)
    */

    (void)src_addr;
    (void)addrlen;

    tcp_t *data = sock->data;
    if (TCP_GET_INFO(data, TCP_CONNECTION_RST_MASK))
        return -ECONNRESET;
    if (data->state == TCP_STATE_CLOSED)
        return -EAGAIN;
    if (data->recv == NULL)
        return 0;
    if (flags & MSG_WAITALL)
        return read_all(sock, buf, len);
    if (flags & MSG_DONTWAIT)
        return read_nonblock(sock, buf, len, flags & MSG_PEEK);
    return read_block(sock, buf, len, flags & MSG_PEEK);
}
