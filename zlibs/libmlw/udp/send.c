#include "../mlw_private.h"

static uint16_t udp_checksum(void *data, int len, uint32_t src_ip, uint32_t dest_ip) {
	uint8_t *udp_data = (uint8_t *)data;
	uint32_t sum = 0;
	sum += (src_ip >> 16) & 0xFFFF;
    sum += src_ip & 0xFFFF;
    sum += (dest_ip >> 16) & 0xFFFF;
    sum += dest_ip & 0xFFFF;
    sum += 17;
    sum += ntohs(len);
    for (int i = 0; i < len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < len)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    return htons(~sum);
}

int mlw_udp_send(mlw_udp_t *inst, void *data, uint16_t len) {
	uint8_t *packet = malloc(len + sizeof(udp_header_t));
	if (!packet)
		return -1;
	memcpy(packet + sizeof(udp_header_t), data, len);
	udp_header_t *hdr = (void *)packet;
	hdr->src_port = htons(inst->src_port);
	hdr->dest_port = htons(inst->dest_port);
	hdr->len = htons(len);
	hdr->checksum = 0;
	hdr->checksum = udp_checksum(packet, len + sizeof(udp_header_t), mlw_info->ip, inst->dest_ip);
	int ret = I_mlw_tcp_send_ip(17, inst->dest_ip, packet, sizeof(udp_header_t) + len);
	free(packet);
	return ret;
}