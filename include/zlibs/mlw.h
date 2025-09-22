#ifndef MLW_H
#define MLW_H

#include <profan/syscall.h>
#include <profan/net.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

typedef struct mlw_packet_t {
	int len;
	void *data;
	uint32_t seq;
	struct mlw_packet_t *next;
} mlw_packet_t;

typedef struct {
	uint32_t next_packet_seq;
	mlw_packet_t *packets;
	uint32_t eth_id;
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t dest_ip;
	uint8_t *buffer;
	int buffer_len;
	uint16_t window; // self
	uint32_t current_seq; // self
} mlw_instance_t;

mlw_instance_t *mlw_open(uint32_t dest_ip, uint16_t dest_port);
int mlw_tcp_close(mlw_instance_t *inst, int timeout_ms);
void *mlw_tcp_recv(mlw_instance_t *inst, int *buffer_len, int timeout_ms);
int mlw_tcp_send(mlw_instance_t *inst, void *data, int len);
uint32_t mlw_ip_from_str(char *ip);

#endif