#ifndef TCP_H
#define TCP_H

#include <modules/socket.h>

#define SOCKET_TCP (AF_INET | (SOCK_STREAM << 8) | (0 << 16 ))

enum {
	TCP_SATE_CLOSED,
    TCP_SATE_LISTEN,
    TCP_SATE_SYN_SENT,
    TCP_SATE_SYN_RECEIVED,
    TCP_SATE_ESTABLISHED,
    TCP_SATE_FIN_WAIT_1,
    TCP_SATE_FIN_WAIT_2,
    TCP_SATE_CLOSE_WAIT,
    TCP_SATE_CLOSING,
    TCP_SATE_LAST_ACK,
    TCP_SATE_TIME_WAIT,
};

typedef struct {
	uint16_t local_port;
	uint16_t rmote_port;
	uint32_t remote_ip;
	uint32_t local_ip;

	uint16_t state;

	struct {
		uint32_t first_seq;
		uint32_t next_seq;
		uint8_t buffer[4096 * 4];
		int buffer_len;
	} recv;
	struct {
		uint32_t first_seq;
		uint32_t next_seq;
		uint8_t buffer[4096 * 4];
		uint8_t wait_seq;
		uint32_t seq_to_wait;
	} send;
} tcp_state_t;



int socket_init_tcp(socket_t *sock);

#endif
