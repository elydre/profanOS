#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>
#include <unistd.h>

#define AF_UNIX     1
#define AF_LOCAL    1
#define AF_INET     2
#define AF_INET6    10
#define AF_PACKET   17
#define AF_NETLINK  16

#define SOCK_STREAM     1
#define SOCK_DGRAM      2
#define SOCK_RAW        3

#define IPPROTO_IP      0
#define IPPROTO_TCP     6
#define IPPROTO_UDP     17

typedef uint32_t socklen_t;
typedef uint16_t sa_family_t;
typedef uint32_t in_addr_t;
typedef uint16_t in_port_t;

struct sockaddr {
	sa_family_t sa_family;
	char sa_data[14];
};


#define INADDR_ANY 0

int socket(int domain, int type, int protocol);
int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
ssize_t sendto(int sockfd, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen);

#endif
