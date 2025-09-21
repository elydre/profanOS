#include "../mlw_private.h"

static int mlw_ip_to_mac(uint32_t ip, uint8_t *mac_res) {
	uint32_t net1 = ip & mlw_info->net_mask;
	uint32_t net2 = mlw_info->ip & mlw_info->net_mask;
	if (net1 != net2) {
		memcpy(mac_res,  mlw_info->router_mac, 6);
		return 0;
	}
	fprintf(stderr, "error: %d.%d.%d.%d local no implemented\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, ip >> 24);
	return 1;
}

static uint16_t ip_checksum(void *buf, size_t len) {
	uint32_t sum = 0;
	uint16_t *w = buf;
	for (size_t i = 0; i < len/2; i++) {
	sum += ntohs(w[i]);
		if (sum > 0xFFFF)
			sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return htons(~sum & 0xFFFF);
}


int mlw_send_ip(uint8_t protocol, uint32_t dest_ip, const uint8_t *data, int len) {
	if (!data || len <= 0)
		return 1;

	uint8_t *packet = malloc(len + sizeof(ip_header_t));
	if (!packet)
		return 1;

	memcpy(&packet[sizeof(ip_header_t)], data, sizeof(uint8_t) * len);

	ip_header_t *header = (ip_header_t *)packet;
	header->v_ihl = 0x45;
	header->tos = 0;
	header->tot_len = htons(len + sizeof(ip_header_t));
	header->id = 0;
	header->flags_n_offset = htons(0x4000);
	header->ttl = 64;
	header->protocol = protocol;
	header->src_ip = mlw_info->ip;
	header->dest_ip = dest_ip;
	header->checksum = 0;
	header->checksum = ip_checksum(packet, sizeof(ip_header_t));

	uint8_t dest_mac[6];
	if (mlw_ip_to_mac(dest_ip, dest_mac)) {
		free(packet);
		return 1;
	}
	int ret = mlw_send_ethernet(0x0800, dest_mac, packet, len + sizeof(ip_header_t));
	free(packet);
	return ret;
}
