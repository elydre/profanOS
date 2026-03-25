/*****************************************************************************\
|   === recv.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../mlw_private.h"

int I_mlw_ip_recv(void **whole_packet, int *whole_len, ip_header_t *header, void **data,
            int *data_len, mlw_tcp_t *inst) {

    int size = modeth_is_ready(inst->eth_id);
    if (size <= 0)
        return 2;
    uint8_t *packet = malloc(size);
    modeth_recv(inst->eth_id, packet);
    if (size < 14) {
        free(packet);
        return 1;
    }
    uint16_t ether_type = *(uint16_t *)&packet[12];
    if (ether_type != htons(0x0800)) {
        free(packet);
        return 1;
    }

    void *ip_addr = packet + 14;
    int ip_len = size - 14;
    if (ip_len < (int)sizeof(ip_header_t)) {
        free(packet);
        return 1;
    }
    ip_header_t *ip_header = (ip_header_t *)ip_addr;
    *data = (uint8_t *)ip_addr + (ip_header->v_ihl & 0xf) * 4;
    *data_len = ip_len - ((uint8_t *)*data - (uint8_t *)ip_addr);
    if (*data_len > ntohs(ip_header->tot_len) - (ip_header->v_ihl & 0xf) * 4)
        *data_len = ntohs(ip_header->tot_len) - (ip_header->v_ihl & 0xf) * 4;
    *whole_packet = packet;
    *whole_len = size;
    *header = *ip_header;
    return 0;
}
