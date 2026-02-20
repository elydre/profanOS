#include "socket.h"
#include <errno.h>

socket_t *sockets = NULL;
int sockets_len = 0;

uint32_t supported[] = {
	SOCKET_TCP,
};

int (*supported_init[])(socket_t *socket) {
	socket_init_tcp,
};


int socket_socket(int domain, int type, int protocol) {
	uint32_t type = domain | (type << 8) | (protocol << 16);
	int i = 0;
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
		sockets[sockets_len].type = SOCKET_TCP;

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
