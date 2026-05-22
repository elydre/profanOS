#include "tcp.h"
#include <modules/filesys.h>
#include <fcntl.h>

static uint32_t rand32() {
	int fd = fm_open("/dev/random", O_RDONLY);
	if (fd < 0)
		return 0xAB38FBE1; // random number be like
	uint32_t res = 0;
	fm_read(fd, &res, 4);
	fm_close(fd);
	return res;
}

int socket_tcp_init(socket_t *sock) {
	tcp_t *info = malloc(sizeof(tcp_t));
	if (!info)
		return 1;
	sock->data = info;

	mem_set(info, 0, sizeof(tcp_t));
	info->state = TCP_STATE_CLOSED;
	info->is_bound = 0;
	info->is_connected = 0;
	info->tosend_len = 0;
	info->recv_len = 0;
	info->first_seq = rand32();
	info->current_seq = info->first_seq;
	return 0;
}
