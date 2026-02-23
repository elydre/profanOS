#include "socket.h"
#include "tcp.h"
#include "udp.h"
#include <errno.h>

socket_t *sockets = NULL;
int sockets_len = 0;

uint32_t supported[] = {
	SOCKET_UDP,
	//SOCKET_TCP,
};

int (*supported_init[])(socket_t *socket) = {
	socket_init_udp,
	//socket_init_tcp,
};


int socket_socket(int domain, int type_, int protocol) {
	uint32_t type = domain | (type_ << 8) | (protocol << 16);
	unsigned int i = 0;
	int res = -EINVAL;
	while (i < sizeof(supported) / sizeof(supported[0])) {
		if (type != supported[i]) {
			i++;
			continue;
		}
		fd_data_t *fd_data = fm_get_free_fd(&res);
		if (fd_data == NULL)
			return -EMFILE;

		sockets = realloc(sockets, sizeof(socket_t) * (sockets_len + 1));
		sockets[sockets_len] = (socket_t){0};
		sockets[sockets_len].type = type;
		sockets[sockets_len].fd = res;
		sockets[sockets_len].ref_count = 0;
		sockets[sockets_len].parent_pid = process_get_pid();

		int err = supported_init[i](&sockets[sockets_len]);
		if (err)
			return err;

		fd_data->sock_id = sockets_len;
		fd_data->type = TYPE_SOCK;

		sockets_len++;
		break;
	}
	return res;
}
