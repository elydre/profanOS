#ifndef MLW_PRIVATE_H
#define MLW_PRIVATE_H

#include "mlw.h"
#include <unistd.h>
#include <stdlib.h>
#include <sys/time.h>

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

typedef struct udp_header_t{
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
} __attribute__((packed)) udp_header_t;

#define TCP_FLAG_CWR (1 << 7)
#define TCP_FLAG_ECE (1 << 6)
#define TCP_FLAG_URG (1 << 5)
#define TCP_FLAG_ACK (1 << 4)
#define TCP_FLAG_PSH (1 << 3)
#define TCP_FLAG_RST (1 << 2)
#define TCP_FLAG_SYN (1 << 1)
#define TCP_FLAG_FIN (1 << 0)

//------------------ tcp
int		I_mlw_tcp_general_send(
			uint16_t src_port, uint32_t dest_ip, uint32_t dest_port,
			uint32_t seq, uint32_t ack, uint8_t flags,
			uint8_t *data, int len,
			uint16_t window
);

uint16_t I_mlw_tcp_checksum(
			uint32_t src_ip, uint32_t dest_ip,
			const uint8_t *tcp_seg, int tcp_len
);

int 	I_mlw_tcp_get_packet_info(
			tcp_recv_info_t *info,
			uint8_t *packet, int len
);

int 	I_mlw_tcp_general_recv(
			tcp_recv_info_t *info,
			mlw_tcp_t *inst, void **whole
);

uint32_t I_mlw_tcp_get_relative(
			uint32_t seq, uint32_t seq_first
);

uint32_t I_mlw_tcp_has_been_seen(
			uint32_t seq,
			uint32_t current_seq, uint32_t seq_first
);

void	I_mlw_tcp_update(
			mlw_tcp_t *inst
);


//------------------ ip
int 	I_mlw_ip_recv(
			void **whole_packet, int *whole_len,
			ip_header_t *header, void **data, int *data_len,
			mlw_tcp_t *inst
);

int		I_mlw_tcp_send_ip(
			uint8_t protocol, uint32_t dest_ip,
			const uint8_t *data, int len
);


//------------------ ethernet
int		I_mlw_tcp_send_ethernet(
			uint16_t type_lit, uint8_t *mac,
			uint8_t *data, int len
);


//------------------ utils
uint32_t	I_mlw_get_time();
uint16_t	I_mlw_rand_u16();
uint32_t	I_mlw_rand_u32();


#endif
