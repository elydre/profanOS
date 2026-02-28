#include <errno.h>
#include "udp.h"

ssize_t socket_recvfrom(recvfrom_arg_t *args) {
	int sockfd = args->sockfd;
	void *buf = args->buf;
	size_t len = args->len;
	int flags = args->flags;
	struct sockaddr *src_addr = args->src_addr;
	socklen_t *addrlen = args->addrlen;
	socket_t *sock = socket_find_fd(sockfd);
	if (!sock)
		return -ENOTSOCK;
	
	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_recvfrom(sock, buf, len, flags, src_addr, addrlen);
		default:
			return -EINVAL;
	}
}
