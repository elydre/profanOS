#include <modules/tcp.h>

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	socket_t *sock = socekt_find_fd(sockfd);
	if (!sock)
		return -ENOTSOCK;
	
	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_sendto(sock, buf, len, flags, dest_addrm addrlen);
		default:
			return -EINVAL;
	}
}
