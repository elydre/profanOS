#include "mlw.h"

typedef struct {
	uint8_t *v_ihl;
	uint8_t *tos;
	uint16_t *tot_len;
	uint16_t *id;
	uint16_t *flags_n_offset;
	uint8_t *ttl;
	uint8_t *protocol;
	uint16_t *checksum;
	uint32_t *src_ip;
	uint32_t *dest_ip;
} ip_header_add_t;

int mlw_send_ip(uint32_t dest_ip, const uint8_t *data, int len) {
	if (!data || len <= 0)
		return 1;

	uint8_t *packet = malloc(sizeof(uint8_t * (len + IP_HEADER_SIZE));
	if (!packet)
		return 1;
	
	memcpy(&packet[IP_HEADER_SIZE], data, sizeof(uint8_t) * len);

	ip_header_add_t header;
	header.v_ihl = (void *)packet;
	header.tos = (void *)&packet[1];
	header.tot_len = (void *)&packet[2];
	header.id = (void *)&packet[4];
	header.flags_n_offset = (void *)&packet[6];
	header.ttl = (void *)&packet[8];
	header.protocol = (void *)&packet[9];
	header.checksum = (void *)&packet[10];
	header.src_ip = (void *)&packet[12];
	header.dest_ip = (void *)&packet[16];

	*header.v_ihl = 0x45;
	*header.tos = 0;
	*header.tot_len = htons(len + IP_HEADER_SIZE);
	*header.id = 0;
	*header.flags_n_offset = htons(0x4000);
	*header.ttl = 64;
	*header.protocol = 17;
	*header.src_ip = mlw->ip;
	*header.dest_ip = dest_ip;

	uint8_t dest_mac[6];
	mlw_ip_to_mac(dest_ip, dest_mac);
	int ret = mlw_send_ethernet(dest_mac, pakcet, len + IP_HEADER_SIZE);
	free(packet);
	return ret;
}
