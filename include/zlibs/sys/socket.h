#ifndef SOCKET_H
#define SOCKET_H

#include <stdint.h>

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

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
};

#endif
