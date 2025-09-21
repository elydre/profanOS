#include "ip-get.h"

void save_router_mac(uint8_t *mac) {
	memcpy(g_info.router_mac, mac, 6);
}

int receive_offer(uint8_t *mac) {
	(void)mac;
	int size = syscall_eth_is_ready(g_eth_id);
	if (size < 0) {
		return 1;
	}
	int packet_size = size;
	uint8_t *packet = malloc(packet_size);
	if (!packet) {
		return 1;
	}
	syscall_eth_recv(g_eth_id, packet);
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
	for (int i = 0; i < opt_len && opt[i] != 0xff; ) {
		if (opt[i] == 0) {
			i++;
			continue;
		}
		if (i + 2 < opt_len) {
			if (opt[i] == 53 && opt[i + 1] == 1 && opt[i + 2] == 2) {
				offered_ip = dhcp->yiaddr;
				dhcp_server_ip = dhcp->siaddr;
				free(packet);
				save_router_mac(eth->src_mac);
				return 0;
			}
		}
		if (i + 1 < opt_len)
			i += opt[i + 1] + 2;
		else
			break;
	}
	free(packet);
	return 1;
}
