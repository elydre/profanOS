/*****************************************************************************\
|   === send_parts.c : 2026 ===                                               |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "tcp.h"
#include <cpu/timer.h>
#include "../include/ip.h"

static uint16_t get_window(tcp_t *data) {
    size_t window = data->recv_max - data->recv_len;
    if (window > 0xFFFF)
        return 0xFFFF;
    return window;
}

uint16_t tcp_checksum(void *data, int len, uint32_t ip_src, uint32_t ip_dest) {
    uint8_t *udp_data = (uint8_t *)data;
    uint32_t sum = 0;
    uint8_t buffer[12];
    mem_copy(buffer, &ip_src, 4);
    mem_copy(buffer + 4, &ip_dest, 4);
    buffer[8] = 0;
    buffer[9] = 6;
    buffer[10] = len >> 8;
    buffer[11] = len & 0xff;
    for (int i = 0; i < 12; i += 2) {
        uint16_t word = buffer[i] << 8;
        word |= buffer[i + 1];
        sum += word;
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    for (int i = 0; i < len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < len)
            word |= udp_data[i + 1];
        sum += word;
        sum = (sum & 0xFFFF) + (sum >> 16);
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    return ~sum;
}

void tcp_send_general(tcp_packet_t *packet) {
    static uint8_t buffer[2048];
    mem_set(buffer, 0, sizeof(buffer));

    buffer[0] = packet->port_src & 0xff;
    buffer[1] = packet->port_src >> 8;
    buffer[2] = packet->port_dest & 0xff;
    buffer[3] = packet->port_dest >> 8;
    buffer[4] = packet->seq >> 24;
    buffer[5] = (packet->seq >> 16) & 0xff;
    buffer[6] = (packet->seq >> 8) & 0xff;
    buffer[7] = packet->seq & 0xff;
    buffer[8] = packet->ack >> 24;
    buffer[9] = (packet->ack >> 16) & 0xff;
    buffer[10] = (packet->ack >> 8) & 0xff;
    buffer[11] = packet->ack & 0xff;
    buffer[12] = (5 << 4) | 0; // data offset
    buffer[13] = packet->flags;
    buffer[14] = packet->window >> 8;
    buffer[15] = packet->window & 0xff;
    buffer[16] = 0;
    buffer[17] = 0;
    buffer[18] = packet->urgent_ptr >> 8;
    buffer[19] = packet->urgent_ptr & 0xff;
    mem_copy(buffer + 20, packet->data, packet->data_len);
    uint16_t checksum = tcp_checksum(buffer, 20 + packet->data_len, packet->ip_src, packet->ip_dest);
    buffer[16] = checksum >> 8;
    buffer[17] = checksum & 0xff;
    socket_on_send_ip(packet->ip_src, packet->ip_dest, 6, buffer, 20 + packet->data_len);
}

void tcp_send_syn(tcp_t *sock) {
    sock->last_send = timer_get_ms();

    tcp_packet_t packet;
    packet.ip_src = sock->local_ip;
    packet.ip_dest = sock->remote_ip;
    packet.port_src = sock->local_port;
    packet.port_dest = sock->remote_port;
    packet.seq = sock->current_seq;
    packet.ack = 0;
    packet.flags = TCP_FLAG_SYN;
    packet.window = 65535;
    packet.urgent_ptr = 0;
    packet.data = NULL;
    packet.data_len = 0;
    tcp_send_general(&packet);
}

void tcp_send_data(tcp_t *sock) {
    sock->last_send = timer_get_ms();

    tcp_packet_t packet;
    packet.ip_src = sock->local_ip;
    packet.ip_dest = sock->remote_ip;
    packet.port_src = sock->local_port;
    packet.port_dest = sock->remote_port;
    packet.seq = sock->current_seq;
    packet.ack = sock->current_ack;
    packet.flags = TCP_FLAG_ACK;
    packet.data_len = TCP_MIN(sock->tosend_len, TCP_MAX_SEND_ONCE);
    if (packet.data_len > 0)
        packet.flags |= TCP_FLAG_PSH;
    if (TCP_GET_INFO(sock, TCP_SEND_FIN_MASK))
        packet.flags |= TCP_FLAG_FIN;
    packet.window = get_window(sock);
    packet.urgent_ptr = 0;
    packet.data = sock->tosend;
    tcp_send_general(&packet);
}

void tcp_send_ack(tcp_t *sock) {
    sock->last_send = timer_get_ms();

    tcp_packet_t packet;
    packet.ip_src = sock->local_ip;
    packet.ip_dest = sock->remote_ip;
    packet.port_src = sock->local_port;
    packet.port_dest = sock->remote_port;
    packet.seq = sock->current_seq;
    packet.ack = sock->current_ack;
    packet.flags = TCP_FLAG_ACK;
    packet.window = get_window(sock);
    packet.urgent_ptr = 0;
    packet.data = NULL;
    packet.data_len = 0;
    tcp_send_general(&packet);
}

void tcp_send_reset(tcp_t *sock) {
    sock->last_send = timer_get_ms();

    tcp_packet_t packet;
    packet.ip_src = sock->local_ip;
    packet.ip_dest = sock->remote_ip;
    packet.port_src = sock->local_port;
    packet.port_dest = sock->remote_port;
    packet.seq = sock->current_seq;
    packet.ack = sock->current_ack;
    packet.flags = TCP_FLAG_RST;
    packet.window = get_window(sock);
    packet.urgent_ptr = 0;
    packet.data = NULL;
    packet.data_len = 0;
    tcp_send_general(&packet);
}
