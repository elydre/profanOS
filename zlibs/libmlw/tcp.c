#include "mlw.h"

struct tcp_header {
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t seq_num;
	uint32_t ack_num;
	uint8_t  data_offset;
	uint8_t  flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_ptr;
} __attribute__((packed));

struct pseudo_header {
	uint32_t src_ip;
	uint32_t dest_ip;
	uint8_t  zero;
	uint8_t  protocol;
	uint16_t tcp_len;
} __attribute__((packed));

typedef struct pseudo_header pseudo_header_t;
typedef struct tcp_header tcp_header_t;

uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip,
						const uint8_t *tcp_seg, int tcp_len) {
	struct pseudo_header psh;
	psh.src_ip = src_ip;
	psh.dest_ip = dest_ip;
	psh.zero = 0;
	psh.protocol = 6;
	psh.tcp_len = htons(tcp_len);

	int total_len = sizeof(psh) + tcp_len;
	uint8_t *buf = malloc(total_len);
	memcpy(buf, &psh, sizeof(psh));
	memcpy(buf + sizeof(psh), tcp_seg, tcp_len);
	uint32_t sum = 0;
	uint16_t *w = (uint16_t *)buf;
	for (int i = 0; i < total_len/2; i++) {
		printf("adddiing %x\n", (uint32_t)htons(w[i]) & 0xFFFF);
	    sum += htons(w[i]);
		if (sum >> 16)
			sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return ~htons(sum) & 0xFFFF;
}


int mlw_tcp_send(uint16_t src_port,
				uint32_t dest_ip, uint32_t dest_port, uint32_t seq,
				uint32_t ack, uint8_t flags, uint8_t *data, int len) {
	int tcp_len = sizeof(struct tcp_header) + len;
	
	uint32_t src_ip = mlw_eth_info->ip;
	uint8_t *packet = malloc(tcp_len);
	if (!packet)
		return 1;
	
	tcp_header_t *tcp = (tcp_header_t *)(void *)packet;
	memset(tcp, 0, sizeof(tcp_header_t));
	tcp->src_port = htons(src_port);
	tcp->dst_port = htons(dest_port);
	tcp->seq_num = htonl(seq);
	tcp->ack_num = htonl(ack);
	tcp->data_offset = (5 << 4);
	tcp->flags = flags;
	tcp->window = htons(65535);
	tcp->urgent_ptr = 0;

	if (data && len > 0)
		memcpy(packet + sizeof(tcp_header_t), data, len);
	tcp->checksum = 0;
	tcp->checksum = tcp_checksum(src_ip, dest_ip, packet, tcp_len);

	int ret = mlw_send_ip(6, dest_ip, packet, tcp_len);
	free(packet);
	return ret;
}
