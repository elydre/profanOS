#ifndef MLW_H
#define MLW_H

#include <profan/syscall.h>
#include <profan/net.h>
#include <stdint.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <string.h>

typedef struct {
	uint8_t is_open;

	uint32_t eth_id;

	uint16_t src_port;
	uint16_t dest_port;
	uint32_t dest_ip;

	struct {
		uint32_t first_seq;
		uint32_t next_seq;
		uint8_t *buffer;
		int buffer_len;
	} recv;
	struct {
		uint32_t first_seq;
		uint32_t next_seq;
	} send;
	uint8_t is_waiting_seq;
	uint32_t seq_to_wait;
} mlw_instance_t;

mlw_instance_t *mlw_open(uint32_t dest_ip, uint16_t dest_port);
int mlw_tcp_close(mlw_instance_t *inst, int timeout_ms);
uint32_t mlw_ip_from_str(char *ip);

int mlw_recv(mlw_instance_t *inst, void *buff, int buff_len, int timeout); // return number of bytes read
int mlw_send(mlw_instance_t *inst, void *buff, int buff_len, int timeout); // returns number of bytes send

#endif