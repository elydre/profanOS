#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include "tcp.h"

typedef struct {
	uint32_t type; // domain | type << 8 | protcol << 16
	union {
		tcp_state_t *tcp;
		void *data;
	};
	int parent_pid;
	int ref_count;
	int fd;
} socket_t;

#define SOCKET_TCP (AF_INET | (SOCK_STREAM << 8) | (0 << 16 ))


extern socket_t *sockets;
extern int sockets_len;

int socket_socket(int domain, int type, int protocol);
int socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int socket_listen(int sockfd, int backlog);
int socket_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

#endif
