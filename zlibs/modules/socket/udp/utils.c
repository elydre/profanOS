/*****************************************************************************\
|   === utils.c : 2026 ===                                                    |
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

uint16_t udp_checksum(void *data, int len, uint32_t src_ip, uint32_t dest_ip) {
    uint8_t *udp_data = (uint8_t *)data;
    uint32_t sum = 0;
    uint8_t buffer[12];
    mem_copy(buffer, &src_ip, 4);
    mem_copy(buffer + 4, &dest_ip, 4);
    buffer[8] = 0;
    buffer[9] = 17;
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

int udp_parse_packet(uint32_t s_ip, uint32_t d_ip, uint8_t *data, int len, udp_packet_t *packet, int verify_checksum) {
    if (len < 8)
        return 1;
    packet->src_ip = s_ip;
    packet->dest_ip = d_ip;
    packet->len = len - 8;
    packet->data = data + 8;
    if (packet->len == 0)
        packet->data = NULL;
    packet->src_port = (uint16_t)data[1] << 8;
    packet->src_port |= data[0];
    packet->dest_port = (uint16_t)data[3] << 8;
    packet->dest_port |= data[2];
    int len2 = (((uint16_t)data[4] << 8) | data[5]) - 8;
    if (len2 > packet->len)
        return 1;
    packet->len = len2;
    if (verify_checksum) {
        uint16_t checksum1 = ((uint16_t)data[7] << 8) | data[8];
        data[7] = 0;
        data[8] = 0;
        uint16_t checksum2 = udp_checksum(data, len, s_ip, d_ip);
        data[7] = checksum1 >> 8;
        data[8] = checksum1 & 0xff;
        if (checksum1 != htons(checksum2))
            return 1;
    }
    return 0;
}
