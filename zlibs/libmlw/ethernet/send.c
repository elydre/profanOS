/*****************************************************************************\
|   === send.c : 2026 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../mlw_private.h"

int I_mlw_tcp_send_ethernet(uint16_t type_lit, uint8_t *mac, uint8_t *data, int len) {
    uint8_t *packet = malloc(6 + 6 + 2 + len);
    if (!packet)
        return 1;
    memcpy(packet, mac, 6);
    memcpy(&packet[6], mlw_info->mac, 6);
    *(uint16_t *)(void *)&packet[12] = htons(type_lit);
    memcpy(&packet[14], data, len);
    int ret = modeth_send(packet, len + 6 + 6 + 2);
    free(packet);
    return ret;
}
