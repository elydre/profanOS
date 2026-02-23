#include <modules/socket.h>
#include <errno.h>
#include "udp.h"

int socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	socket_t *sock = NULL;
	for (int i = 0; i < sockets_len; i++) {
		if (sockets[i].fd == sockfd) {
			sock = &sockets[i];
			break;
		}
	}
	if (!sock)
		return -ENOTSOCK;

	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_connect(sock, addr, addrlen);
		default:
			return -EINVAL;
	}
}
