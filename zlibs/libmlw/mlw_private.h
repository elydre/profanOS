#ifndef MLW_PRIVATE_H
#define MLW_PRIVATE_H

#include "mlw.h"

extern eth_info_t *mlw_info;

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
} __attribute__((packed)) ip_header_t;

typedef struct tcp_header_t {
	uint16_t src_port;
	uint16_t dst_port;
	uint32_t seq_num;
	uint32_t ack_num;
	uint8_t  data_offset;
	uint8_t  flags;
	uint16_t window;
	uint16_t checksum;
	uint16_t urgent_ptr;
} __attribute__((packed)) tcp_header_t;

typedef struct pseudo_header_t {
	uint32_t src_ip;
	uint32_t dest_ip;
	uint8_t  zero;
	uint8_t  protocol;
	uint16_t tcp_len;
} __attribute__((packed)) pseudo_header_t;

typedef struct tcp_recv_info_t {
	uint32_t src_ip;
	uint32_t dest_ip;
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t seq;
	uint32_t ack;
	uint8_t flags;
	uint16_t window;
	uint8_t *data;
	int len;
} tcp_recv_info_t;


int mlw_tcp_general_send(uint16_t src_port,
				uint32_t dest_ip, uint32_t dest_port, uint32_t seq,
				uint32_t ack, uint8_t flags, uint8_t *data, int len, uint16_t window);

int mlw_ip_recv(void **whole_packet, int *whole_len, ip_header_t *header, void **data, int *data_len, mlw_instance_t *inst);
int mlw_send_ethernet(uint16_t type_lit, uint8_t *mac, uint8_t *data, int len);
int mlw_send_ip(uint8_t protocol, uint32_t dest_ip, const uint8_t *data, int len);
uint32_t get_time();
int tcp_get_packet_info(tcp_recv_info_t *info, uint8_t *packet, int len);

uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip,
						const uint8_t *tcp_seg, int tcp_len);

#endif
