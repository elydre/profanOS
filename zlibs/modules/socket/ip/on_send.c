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

#include "ip.h"

static uint16_t ip_checksum(void *buf, int len) {
    uint32_t sum = 0;
    uint16_t *w = buf;
    for (int i = 0; i < len / 2; i++) {
        sum += ntohs(w[i]);
        if (sum > 0xFFFF)
            sum = (sum & 0xFFFF) + (sum >> 16);
    }
    return ~sum & 0xFFFF;
}

void socket_on_send_ip(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, uint8_t *data, int data_len) {
    if (data_len > 1500 - 20 || data_len < 0) {
        kprintf("WARNING IP PACKET TOO LONG %d\n", data_len);
        return ;
    }
    static uint8_t eth_buffer[1500 + 6 + 6 + 2]; // static to not explode
    uint8_t *buffer = &eth_buffer[6 + 6 + 2];
    buffer[0] = 0x45;
    buffer[1] = 0;
    int total_len = data_len + 20;
    buffer[2] = total_len >> 8;
    buffer[3] = total_len & 0xff;
    buffer[4] = 0;
    buffer[5] = 0;
    buffer[6] = 0x40;
    buffer[7] = 0x00;
    buffer[8] = 64;
    buffer[9] = 17;
    buffer[10] = 0;
    buffer[11] = 0;
    mem_copy(&buffer[12], &src_ip, 4);
    mem_copy(&buffer[16], &dest_ip, 4);
    uint16_t checksum = ip_checksum(buffer, 20);
    buffer[10] = checksum >> 8;
    buffer[11] = checksum & 0xff;
    mem_copy(&buffer[20], data, data_len);

    eth_buffer[12] = 0x08;
    eth_buffer[13] = 0x00;

    mem_copy(&eth_buffer[6], &eth_info.mac, 6);
    if ((src_ip & eth_info.net_mask) == (dest_ip & eth_info.net_mask))
        mem_copy(eth_buffer, "\xff\xff\xff\xff\xff\xff", 6);
    else
        mem_copy(eth_buffer, &eth_info.router_mac, 6);
    eth_send(eth_buffer, 6 + 6 + 2 + 20 + data_len);
}
