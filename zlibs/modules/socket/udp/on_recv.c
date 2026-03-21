/*****************************************************************************\
|   === on_recv.c : 2026 ===                                                  |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "udp.h"
#include "utils.h"

void socket_on_recv_udp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len) {
    udp_packet_t packet = {0};
    if (udp_parse_packet(src_ip, dest_ip, data, data_len, &packet, 1)) {
        return ;
    }
    for (int i = 0; i < sockets_len; i++) {
        if (sockets[i].type != SOCKET_UDP)
            continue;
        udp_t *data = sockets[i].data;
        if (!data->is_bound)
            continue;
        if (data->local_port != packet.dest_port)
            continue;
        if (data->local_ip != 0 && data->local_ip != packet.dest_ip)
            break;
        if (data->is_connected && data->remote_ip != packet.src_ip && data->remote_port != packet.src_port)
            break;
        if (data->recv_len == 64)
            break;
        data->recv[data->recv_len].src_ip = packet.src_ip;
        data->recv[data->recv_len].dest_ip = packet.dest_ip;
        data->recv[data->recv_len].src_port = packet.src_port;
        data->recv[data->recv_len].dest_port = packet.dest_port;
        data->recv[data->recv_len].len = packet.len;
        data->recv[data->recv_len].data = malloc(packet.len);
        mem_copy(data->recv[data->recv_len].data, packet.data, packet.len);
        data->recv_len++;
        break;
    }
}
