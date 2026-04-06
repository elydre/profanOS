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
	if (flags & MSG_DONTWAIT && data->recv_len == 0)
		return -EAGAIN;
    while (data->recv_len == 0)
        process_sleep(process_get_pid(), 5);

    if ((size_t)data->recv[0].len < len)
        len = data->recv[0].len;
    mem_copy(buf, data->recv[0].data, len);
    if (src_addr && addrlen) {
        struct sockaddr_in *addr2 = (void *)src_addr;
        addr2->sin_family = AF_INET;
        addr2->sin_port = data->recv[0].src_port;
        addr2->sin_addr.s_addr = data->recv[0].src_ip;
        *addrlen = sizeof(struct sockaddr_in);
    }
    mem_copy(data->recv, &data->recv[1], sizeof(udp_packet_t) * (data->recv_len - 1));
    if (!(flags & MSG_PEEK))
        data->recv_len--;
    return (ssize_t) len;
}
