/*****************************************************************************\
|   === send_parts.c : 2026 ===                                               |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "tcp.h"



void tcp_send_general(tcp_packet_t *packet) {
    static uint8_t buffer[2048];
    
    buffer[0] = packet->src_port >> 8;
    buffer[1] = packet->src_port & 0xff;
    buffer[2] = packet->dest_port >> 8;
    buffer[3] = packet->dest_port & 0xff;
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
    uint16_t checksum = tcp_compute_checksum(packet);
    buffer[16] = checksum >> 8;
    buffer[17] = checksum & 0xff;
    

    buffer[18] = packet->urgent >> 8;
    buffer[19] = packet->urgent & 0xff;
    memcpy(buffer + 20, packet->data, packet->data_len);
    ip_send(packet->src_ip, packet->dest_ip, IP_PROTO_TCP, buffer, 20 + packet->data_len);
}

void tcp_send_syn(tcp_t *sock) {
    // !TODO implement this
    sock->last_send = now;
}

void tcp_send_data(tcp_t *sock) {
    // !TODO implement this
    sock->last_send = now;
}

void tcp_send_ack(tcp_t *sock) {
    // !TODO implement this
    sock->last_send = now;
}

void tcp_send_reset(tcp_t *sock) {
    // !TODO implement this
    sock->last_send = now;
}
