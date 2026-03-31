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

#include <kernel/process.h>
#include <minilib.h>

#include "udp.h"

ssize_t socket_udp_recvfrom(socket_t *sock, void *buf, size_t len, int flags,
            struct sockaddr *src_addr, socklen_t *addrlen) {

    udp_t *data = sock->data;
    while (data->recv_len == 0)
        process_sleep(process_get_pid(), 10);
    if ((size_t)data->recv[data->recv_len - 1].len < len)
        len = data->recv[data->recv_len - 1].len;
    mem_copy(buf, data->recv[data->recv_len - 1].data, len);
    if (src_addr && addrlen) {
        struct sockaddr_in *addr2 = (void *)src_addr;
        addr2->sin_family = AF_INET;
        addr2->sin_port = data->recv[data->recv_len - 1].src_port;
        addr2->sin_addr.s_addr = data->recv[data->recv_len - 1].src_ip;
        *addrlen = sizeof(struct sockaddr_in);
    }
    data->recv_len--;
    return (ssize_t) len;
}
