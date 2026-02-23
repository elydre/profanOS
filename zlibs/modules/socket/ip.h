#ifndef IP_H
#define IP_H

#include "socket.h"

typedef struct ip_header_t {
	uint8_t  v_ihl;
	uint8_t  tos;
	uint16_t tot_len;
	uint16_t id;
	uint16_t flags_n_offset;
	uint8_t  ttl;
	uint8_t  protocol;
	uint16_t checksum;
	uint32_t src_ip;
	uint32_t dest_ip;
	uint8_t *options;
	int options_len;
	uint8_t *data;
	int data_len;
} ip_header_t;

typedef struct pseudo_header_t {
	uint32_t src_ip;
	uint32_t dest_ip;
	uint8_t  zero;
	uint8_t  protocol;
	uint16_t tcp_len;
} __attribute__((packed)) pseudo_header_t;

void socket_on_recv_ip4(int len, uint8_t *packet);
void socket_ip_send(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, uint8_t *data, int data_len);

#endif
