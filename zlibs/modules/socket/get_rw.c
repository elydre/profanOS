#include "udp.h"

int socket_get_rw(int id) {
	socket_t *sock = socket_find_id(id);
	if (!sock)
		return -1;
	
	switch (sock->type) {
		case SOCKET_UDP:
			return socket_udp_get_rw(sock);
		default:
			return -1;
	}
}
