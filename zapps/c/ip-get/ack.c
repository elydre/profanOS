#include "ip-get.h"

int receive_ack(uint8_t *mac) {
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
	for (int i = 0; i < opt_len && opt[i] != 0xff; ) {
		if (opt[i] == 0) {
			i++;
			continue;
		}
		if (i + 2 < opt_len) {
			if (opt[i] == 53 && opt[i + 1] == 1 && opt[i + 2] == 5) {
				free(packet);
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
