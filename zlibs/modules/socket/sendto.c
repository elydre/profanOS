#include <errno.h>
#include "udp.h"

ssize_t socket_sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	socket_t *sock = socket_find_fd(sockfd);
	if (!sock)
		return -ENOTSOCK;
	
	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_sendto(sock, buf, len, flags, dest_addr, addrlen);
		default:
			return -EINVAL;
	}
}
