
#include <ktype.h>
#include <drivers/e1000.h>
#include <minilib.h>
#include <drivers/ethernet.h>
#include <cpu/timer.h>

uint8_t eth_mac[6] = {0};
uint8_t eth_ip[4] = {0};

uint32_t eth_get_transactiob_id() {
	static uint32_t id = 0;
    if (id == 0)
        id = timer_get_ms();
    return id++;
}

void build_dhcp_packet(uint8_t *buffer, uint8_t *mac);

int eth_init(void) {
	if (e1000_is_inited)
		e1000_set_mac(eth_mac);

	uint8_t *dhcp = malloc(292);
	build_dhcp_packet(dhcp, eth_mac);
	eth_send_packet(dhcp, 292);
	free(dhcp);
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

void eth_send_arp(uint8_t src_ip[4], uint8_t dst_ip[4]) {
	arp_packet_t packet = {0};
	packet.htype = 1;
	packet.ptype = ETHERNET_TYPE_IPV4;
	packet.hlen = 6;
	packet.plen = 4;
	packet.opcode = 1;
	mem_copy(packet.srchw, eth_mac, 6);
	mem_copy(packet.srcpr, src_ip, 4);
	mem_copy(packet.dsthw, eth_mac, 6);
	mem_copy(packet.dstpr, dst_ip, 4);
	e1000_send_packet(&packet, sizeof(arp_packet_t));
}

void eth_send_dhcp_discover() {

}

void build_dhcp_discover(uint8_t *buffer, uint8_t *mac) {
    struct dhcp_header *dhcp = (struct dhcp_header *)buffer;
    mem_set(dhcp, 0, sizeof(struct dhcp_header));

    dhcp->op = 1;  // Request
    dhcp->htype = 1;  // Ethernet
    dhcp->hlen = 6;  // Taille adresse MAC
    dhcp->xid = eth_get_transactiob_id();
    dhcp->flags = htons(0x8000);  // Broadcast
    mem_copy(dhcp->chaddr, mac, 6);  // Adresse MAC du client
    dhcp->magic_cookie = htonl(0x63825363);  // Magic cookie

    // Ajout des options DHCP
    uint8_t *options = dhcp->options;
    *(options)++ = 53; // Option: DHCP Message Type
    *(options)++ = 1;  // Longueur
    *(options)++ = 1;  // DHCPDISCOVER

    *(options)++ = 55; // Option: Parameter Request List
    *(options)++ = 3;  // Longueur
    *(options)++ = 1;  // Subnet Mask
    *(options)++ = 3;  // Router
    *(options)++ = 6;  // DNS Server

    *(options)++ = 255; // End option
}

uint16_t ip_checksum(void *vdata, uint32_t length) {
    uint16_t *data = vdata;
    uint32_t sum = 0;

    for (; length > 1; length -= 2) {
        sum += *data++;
    }

    if (length == 1) {
        sum += *(uint8_t *)data;
    }

    while (sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~sum;
}

void build_dhcp_packet(uint8_t *buffer, uint8_t *mac) {
    struct eth_header *eth = (struct eth_header *)buffer;
    struct ip_header *ip = (struct ip_header *)(buffer + sizeof(struct eth_header));
    struct udp_header *udp = (struct udp_header *)(buffer + sizeof(struct eth_header) + sizeof(struct ip_header));
    uint8_t *dhcp = buffer + sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header);

    // Ethernet
    mem_set(eth->dst_mac, 0xFF, 6);  // Broadcast
    mem_copy(eth->src_mac, mac, 6);
    eth->ethertype = htons(ETHERNET_TYPE_IPV4);

    // IP
    ip->version = 4;
    ip->ihl = 5;
    ip->tos = 0;
    ip->total_length = htons(sizeof(struct ip_header) + sizeof(struct udp_header) + sizeof(struct dhcp_header) + 10);
    ip->id = 0;
    ip->flags_fragment = 0;
    ip->ttl = 64;
    ip->protocol = IP_PROTO_UDP;
    ip->src_ip = 0;  // 0.0.0.0
    ip->dst_ip = htonl(0xFFFFFFFF);  // 255.255.255.255
    ip->checksum = ip_checksum(ip, sizeof(struct ip_header));

    // UDP
    udp->src_port = htons(UDP_PORT_DHCP_CLIENT);
    udp->dst_port = htons(UDP_PORT_DHCP_SERVER);
    udp->length = htons(sizeof(struct udp_header) + sizeof(struct dhcp_header) + 10);
    udp->checksum = 0;  // Pas obligatoire

    // DHCP
    build_dhcp_discover(dhcp, mac);
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

static eth_device_t devices[256] = {0}; // [0] is null device
static int devices_current_len = 1;

uint8_t eth_new_device(char *name, pci_device_t *pci, uint8_t mac[6], void (*send)());

void eth_append_packet(void *data, uint32_t len, uint8_t eth_id) {

}