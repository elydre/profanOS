#include <modules/socket.h>
#include <errno.h>
#include "udp.h"

int socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	socket_t *sock = socket_find_fd(sockfd);
	if (!sock)
		return -ENOTSOCK;

	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_connect(sock, addr, addrlen);
		default:
			return -EINVAL;
	}
}
