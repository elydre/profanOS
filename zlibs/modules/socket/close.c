#include <errno.h>
#include "udp.h"

int socket_close(socket_t *sock) {
	sock->ref_count--;
	if (sock->ref_count)
		return 0;
	sock->parent_pid = -1;
	sock->fd = -1;
	return 0;
}

int socket_close_fd(int fd) {
	socket_t *sock = socket_find_fd(fd);
	if (!sock)
		return -EINVAL;
	return socket_close(sock);
}
