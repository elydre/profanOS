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

uint32_t eth_get_transactiob_id() {
    static uint32_t id = 0;
    if (id == 0)
        id = timer_get_ms();
    return id++;
}


static void get_ip_address();

int eth_init(void) {
    if (e1000_is_inited)
        e1000_set_mac(eth_mac);
    get_ip_address();
    return 2;
}

void eth_send_packet(const void * p_data, uint16_t p_len) {
    kprintf_serial("debug1\n");
    if (e1000_is_inited) {
        kprintf_serial("debug2\n");
        e1000_send_packet(p_data, p_len);
        kprintf_serial("debugEND\n");
    }
    else
        kprintf("eth_send_packet no device found inited\n");
}

static uint16_t swap_endian16(uint16_t x) {
    return ((x & 0xff) << 8) | ((x >> 8) & 0xff);
}

static uint32_t swap_endian32(uint32_t x) {
    uint32_t res = 0;
    res |= (x & 0xff) << 24 ;
    res |= ((x >> 8) & 0xff) << 16;
    res |= ((x >> 16) & 0xff) << 8;
    res |= ((x >> 24) & 0xff);
    return res;
}

uint16_t htons(uint16_t x) {
    return swap_endian16(x);
}

uint32_t htonl(uint32_t x) {
    return swap_endian32(x);
}

static int get_ip_state = 0;
static uint8_t get_ip_proposal[4] = {0};

void get_ip_handler(const eth_raw_packet_t *packet) {
    if (get_ip_state == 0) {
        uint8_t *data = packet->data;
        uint16_t len = packet->len;

        if (len < sizeof(eth_header_t))
            return ;
        if (mem_cmp(eth_mac, ((eth_header_t *)data)->dst_mac) != 0)
            return ;
        if (htons(((eth_header_t *)data)->ethertype) != 0x0800)
            return ;
        len -= sizeof(eth_header_t);
        data += sizeof(eth_header_t);
        if (len < sizeof(ip_header_t))
            return ;

        ip_header_t *ip_header = (ip_header_t *)data;
        if (ip_header->protocol != IP_PROTO_UDP)
            return ;
        len -= sizeof(ip_header_t);
        data += sizeof(ip_header_t);
        if (len < sizeof(udp_header_t))
            return ;
        udp_header_t *udp_header = (udp_header_t *)data;
        if (udp_header->src_port != htons(67) || udp_header->dst_port != htons(68))
            return ;
        uint16_t dhcp_len = htons(udp_header->length) - sizeof(udp_header_t);
        len -= sizeof(udp_header_t);
        data += sizeof(udp_header_t);
        if (len < dhcp_len || len < 32)
            return ;
        if (data[0] != 0x02 || data[1] != 0x01 || data[2] != 0x06 || data[3] != 0)
            return ;
        // skip 4 Bytes for XID
        data += 8;
        if (*(uint16_t *)data  != 0 || *(uint16_t *)(data + 2)  != 0)
            return ;
        data += 4;
        if (*(uint32_t *)data != 0)
            return ;
        data += 4;
        mem_copy(get_ip_proposal, data, 4);
        get_ip_state = 1;
    }



}


static void get_ip_address() {
    eth_register_handler(&get_ip_handler);

    uint8_t *dhcp = malloc(292);
    build_dhcp_packet(dhcp, eth_mac);
    eth_send_packet(dhcp, 292);
    free(dhcp);

    uint32_t t0 = timer_get_ms();
    while (get_ip_state != 1 && timer_get_ms() < t0 + 1000)
        ;
    if (get_ip_state != 1) {
        kprintf("Could not find IP address\n");
        return ;
    }
    kprintf("ip address proposal %d.%d.%d.%d\n", get_ip_proposal[0], get_ip_proposal[1], get_ip_proposal[2], get_ip_proposal[3]);

    eth_remove_handlers(&get_ip_handler);
}

