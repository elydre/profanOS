#ifndef UDP_H
#define UDP_H

#include "socket.h"

#define SOCKET_UDP (AF_INET | (SOCK_DGRAM << 8) | (0 << 16 ))

typedef struct udp_header_t{
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t len;
	uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
	uint32_t src_ip;
	uint32_t dest_ip;
	uint32_t dest_port;
	uint32_t src_port;
	int len;
	uint8_t *data;
} udp_packet_t;

typedef struct {
	uint32_t local_ip; // any
	uint32_t local_port;
	uint8_t is_bound;

	uint32_t remote_ip;
	uint32_t remote_port;
	uint8_t is_connected;

	udp_packet_t recv[64];
	int recv_len;
	udp_packet_t send[64];
	int send_len;
} udp_t;

int socket_init_udp(socket_t *sock);
void socket_on_recv_udp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len);

void socket_udp_tick(socket_t *sock);

#endif
