/*****************************************************************************\
|   === sendto.c : 2026 ===                                                   |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/eth.h>
#include <minilib.h>
#include "udp.h"

ssize_t socket_udp_send(socket_t *sock, const uint8_t *buffer, size_t len, uint32_t dip, uint16_t dport) {
    udp_t *data = sock->data;
    if (len > 1450)
        return -EMSGSIZE;
    if (data->send_len == 64)
        return -ENOMEM;
    udp_packet_t *packet = &data->send[data->send_len];
    if (dip == 0) {
        if (!data->is_connected)
            return -EINVAL;
        packet->dest_ip = data->remote_ip;
        packet->dest_port = data->remote_port;
    }
    else {
        packet->dest_ip = dip;
        packet->dest_port = dport;
    }

    eth_info_t info;
    eth_get_info(0, &info);

    if (!data->is_bound) {
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = info.ip;
        addr.sin_port = 0;

        int err = socket_udp_bind(sock, (void *)&addr, sizeof(addr));
        if (err)
            return err;
    }
    packet->src_ip = data->local_ip;
    packet->src_port = data->local_port;
    packet->len = len;
    packet->data = malloc(len);
    mem_copy(packet->data, buffer, len);
    data->send_len++;
    return len;
}

ssize_t socket_udp_sendto(socket_t *sock, const void *buf, size_t len, int flags,
            const struct sockaddr *dest_addr, socklen_t addrlen) {

    if (!dest_addr && addrlen == 0)
        return socket_udp_send(sock, buf, len, 0, 0);

    if (addrlen != sizeof(struct sockaddr_in))
        return -EINVAL;
    const struct sockaddr_in *addr2 = (void *)dest_addr;
    if (addr2->sin_family != AF_INET)
        return -EINVAL;
    if (addr2->sin_port == 0)
        return -EINVAL;
    return socket_udp_send(sock, buf, len, addr2->sin_addr.s_addr, addr2->sin_port);
}
