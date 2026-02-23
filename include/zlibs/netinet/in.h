#ifndef NETINET_IN_H
#define NETINET_IN_H

#include <sys/socket.h>

struct in_addr {
	in_addr_t s_addr;
};

struct sockaddr_in {
	sa_family_t sin_family;
	in_port_t sin_port;
	struct in_addr sin_addr;
};

#endif
