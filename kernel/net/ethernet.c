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
#include <net/dhcp.h>
#include <drivers/e1000.h>
#include <minilib.h>
#include <cpu/timer.h>

uint8_t eth_mac[6] = {0};
uint8_t eth_ip[4] = {0};

static eth_raw_packet_t *packets = NULL;
static int packets_len = 0;
static int packets_alloc = 0;

uint32_t eth_get_transactiob_id() {
    static uint32_t id = 0;
    if (id == 0)
        id = timer_get_ms();
    return id++;
}


static void get_ip_address() {
    uint8_t *dhcp = malloc(292);
    build_dhcp_packet(dhcp, eth_mac);
    eth_send_packet(dhcp, 292);
    free(dhcp);
}

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

// data will be copied
void eth_append_packet(void *data, uint32_t len) {
    if (packets_len == packets_alloc) {
        packets_alloc += 10;
        packets = realloc_as_kernel(packets, sizeof(eth_raw_packet_t *) * packets_alloc);
    }
    eth_raw_packet_t *packet = &packets[packets_len];
    packet->data = malloc(len);
    mem_copy(packet->data, data, len);
    packets_len++;
    packet->data = 0;
}

void eth_destroy_packet(eth_raw_packet_t *packet) {
    free(packet->data);
}

eth_raw_packet_t *eth_pop_oldest_packet() {
    if (packets_len <= 0)
        return NULL;

    eth_raw_packet_t *res = &packets[0];
    for (int i = 1; i < packets_len; i++)
        packets[i - 1] = packets[i];
    packets_len++;
    return res;
}
