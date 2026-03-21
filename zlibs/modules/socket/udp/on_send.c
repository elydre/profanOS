/*****************************************************************************\
|   === on_send.c : 2026 ===                                                  |
|                                                                             |
|    Unix socket implementation as kernel module                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "udp.h"
#include "ip.h"
#include "utils.h"

void socket_on_send_udp(udp_packet_t *packet) {
    static uint8_t buffer[1500 - 20];

    buffer[0] = packet->src_port & 0xff;
    buffer[1] = packet->src_port >> 8;
    buffer[2] = packet->dest_port & 0xff;
    buffer[3] = packet->dest_port >> 8;

    int len = packet->len + 8;
    buffer[4] = len >> 8;
    buffer[5] = len & 0xff;
    buffer[6] = 0;
    buffer[7] = 0;
    mem_copy(&buffer[8], packet->data, len - 8);
    uint16_t checksum = udp_checksum(buffer, len, packet->src_ip, packet->dest_ip);
    buffer[6] = checksum >> 8;
    buffer[7] = checksum & 0xff;

    socket_on_send_ip(packet->src_ip, packet->dest_ip, 17, buffer, 8 + packet->len);
}
