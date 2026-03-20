#include <modules/socket.h>
#include "udp.h"

protocol_t socket_protocols[] = {
	{
		SOCKET_UDP,
		socket_udp_init,
		socket_udp_bind,
		socket_udp_connect,
		socket_udp_sendto,
		socket_udp_recvfrom
	},
	{0},
};

protocol_t *socket_find_protocol(uint32_t type) {
	for (size_t i = 0; socket_protocols[i].type; i++) {
		if (socket_protocols[i].prot == type)
			return &socket_protocols[i];
	}
	return NULL;
}
