#include <sys/socket.h>
#include <errno.h>
#include <modules/socket.h>
#include <stdio.h>

int socket(int domain, int type, int protocol) {
	int ret = socket_socket(domain, type, protocol);
	printf("socket ret %d\n", ret);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int ret = socket_bind(sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen) {
	int ret = socket_connect(sockfd, addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	int ret = socket_sendto(sockfd, buf, len, flags, dest_addr, addrlen);
	if (ret < 0) {
		errno = -ret;
		ret = -1;
	}
	return ret;
}
