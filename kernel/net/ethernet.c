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


#include <minilib.h>
#include <cpu/timer.h>
#include <kernel/scubasuit.h>
#include <net.h>
#include <drivers/e1000.h>
#include <drivers/rtl8168.h>

static uint32_t get_phys(const void *ptr) {
    uint32_t addr_phys = (uint32_t)ptr;
	addr_phys -= addr_phys % 4096;
	addr_phys = (uint32_t)scuba_call_phys((void *)addr_phys);
	addr_phys += (uint32_t)ptr % 4096;
    return addr_phys;
}

eth_info_t eth_info = (eth_info_t){0};

int eth_init(void) {
    if (e1000_is_inited)
        e1000_set_mac((uint8_t *)eth_info.mac);
    if (rtl8168_is_inited)
        rtl8168_set_mac((uint8_t *)eth_info.mac);
    return 2;
}

int eth_send_packet(const void * addr, uint16_t p_len) {
    if (p_len < 6)
        return 1;
    uint32_t addr_phys = get_phys(addr);

    int dest_type = 0;
    if (!mem_cmp(addr, (uint8_t *)&eth_info.mac, 6))
        dest_type = 1;
    if (!mem_cmp(addr, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
        dest_type = 2;

    if (dest_type)
        eth_recv_packet(addr, p_len);
    if (dest_type == 1)
        return 0;
    if (e1000_is_inited)
        return e1000_send_packet((const void *)addr_phys, p_len);
    else if (rtl8168_is_inited)
        return rtl8168_send_packet((const void *)addr, p_len);
    kprintf("eth_send_packet no device found inited\n");
    return 1;
}

void eth_recv_packet(const void *addr, uint16_t p_len) {
    eth_listeners_add_packet(addr, (int)p_len);
}
