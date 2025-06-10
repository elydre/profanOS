/*****************************************************************************\
|   === ethernet.c : 2025 ===                                                 |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <net/ethernet.h>
#include <net/ip.h>
#include <net/dhcp.h>
#include <drivers/e1000.h>
#include <minilib.h>
#include <cpu/timer.h>

uint8_t eth_mac[6] = {0};
uint8_t eth_ip[4] = {0};

int eth_init(void) {
    if (e1000_is_inited)
        e1000_set_mac(eth_mac);
    return 2;
}

void eth_send_packet(const void * p_data, uint16_t p_len) {
    if (e1000_is_inited) {
        e1000_send_packet(p_data, p_len);
    }
    else
        kprintf("eth_send_packet no device found inited\n");
}
