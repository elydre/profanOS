#include "ip-get.h"

int receive_offer(uint8_t *mac) {
	(void)mac;
	if (!syscall_eth_listen_isready()) {
		return 1;
	}
	int packet_size = syscall_eth_listen_getsize();
	uint8_t *packet = malloc(packet_size);
	if (!packet) {
		return 1;
	}
	syscall_eth_listen_get(packet);
	ethernet_header_t *eth = (ethernet_header_t *)packet;
	if (eth->ether_type != htons(0x0800)) {
		free(packet);
		return 1;
	}

	ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
	if (ip->protocol != 17) {
		free(packet);
		return 1;
	}

	udp_header_t *udp = (udp_header_t *)((uint8_t *)ip + sizeof(ip_header_t));
	if (ntohs(udp->src_port) != 67 || ntohs(udp->dest_port) != 68) {
		free(packet);
		return 1;
	}

	dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)((uint8_t *)udp + sizeof(udp_header_t));
	if (dhcp->op != 2 || dhcp->xid != last_xid) {
		free(packet);
		return 1;
	}

	uint8_t *opt = (uint8_t *)(dhcp + 1);
	if (opt >= packet + packet_size) {
		free(packet);
		return 1;
	}
	int opt_len = packet_size - (opt - packet);
	int i = 0;
	while (i < opt_len) {
		if (*opt == 0xFF)
			break;
		if (*opt == 53 && *(opt + 1) == 1 && *(opt + 2) == 2) { // DHCPOFFER
			offered_ip = dhcp->yiaddr;
			free(packet);
			dhcp_server_ip = dhcp->siaddr;
			return 0;
		}
		opt += *(opt + 1) + 2;
		i += *(opt + 1) + 2;
	}
	free(packet);
	return 1;
}
