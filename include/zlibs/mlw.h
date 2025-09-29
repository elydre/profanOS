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
} mlw_tcp_t;

typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint32_t dest_ip;
	uint32_t eth_id;
} mlw_udp_t;

// init
#define MLW_INIT_RAND (1 << 0) // does mlw need to init rand with srand(time(NULL)) ?

int mlw_init(uint32_t flags);
void mlw_end();


// ip
uint32_t	mlw_ip_from_str(char *ip);


// tcp
mlw_tcp_t	*mlw_tcp_open(
				uint32_t dest_ip,
				uint16_t dest_port
);

int 		mlw_tcp_close(
				mlw_tcp_t *inst
);

int 		mlw_tcp_recv(
				mlw_tcp_t *inst,
				void *buff, int buff_len,
				int timeout // -1: inf, 0: return if no packet
); // return number of bytes read

int 		mlw_tcp_send(
				mlw_tcp_t *inst,
				void *data, int len
); // returns number of bytes send


// udp
mlw_udp_t	*mlw_udp_open(uint32_t dest_ip, uint16_t dest_port);
int			mlw_udp_send(mlw_udp_t *inst, void *data, uint16_t len);

#endif
