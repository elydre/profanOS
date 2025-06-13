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
#include <drivers/rtl8168.h>
#include <minilib.h>
#include <cpu/timer.h>
#include <kernel/scubasuit.h>

uint8_t eth_mac[6] = {0};
uint8_t eth_ip[4] = {0};

int eth_init(void) {
    if (e1000_is_inited)
        e1000_set_mac(eth_mac);
    if (rtl8168_is_inited)
        rtl8168_set_mac(eth_mac);
    return 2;
}

void eth_send_packet(const void * addr, uint16_t p_len) {
    uint32_t addr_phys = (uint32_t)addr;
	addr_phys -= addr_phys % 4096;
	addr_phys = (uint32_t)scuba_call_phys((void *)addr_phys);
	addr_phys += (uint32_t)addr % 4096;
    if (e1000_is_inited)
        e1000_send_packet((void *)addr_phys, p_len);
    else if (rtl8168_is_inited)
        rtl8168_send_packet((void *)addr, p_len);
    else
        kprintf("eth_send_packet no device found inited\n");
}
