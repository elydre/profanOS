#include "tcp.h"
#include <kernel/process.h>
#include <errno.h>

ssize_t socket_tcp_recvfrom(socket_t *sock, void *buf, size_t len, int flags, struct sockaddr *src_addr, socklen_t *addrlen) {
	(void)src_addr;
	(void)addrlen;
	tcp_t *data = sock->data;

	if (data->state == TCP_STATE_CLOSED)
		return -EAGAIN;
	while (data->recv_len == 0) {
		if (data->state != TCP_STATE_OPEN)
			break;
		process_sleep(process_get_pid(), 5);
	}
	if (data->recv_len == 0)
		return 0;
	ssize_t to_read = TCP_MIN(data->recv_len, len);
	mem_copy(buf, data->recv, to_read);
	data->recv_len -= to_read;
	mem_copy(data->recv, &data->recv[to_read], data->recv_len);
	
	return to_read;
}
