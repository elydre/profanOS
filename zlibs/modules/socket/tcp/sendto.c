#include "tcp.h"
#include <kernel/process.h>
#include <errno.h>

ssize_t socket_tcp_sendto(socket_t *sock, const void *buf, size_t len, int flags,
            const struct sockaddr *dest_addr, socklen_t addrlen) {
	if (addrlen != 0 || dest_addr != 0)
		return -EISCONN;
	return socket_tcp_send(sock, buf, len, flags);
}

ssize_t socket_tcp_send(socket_t *sock, const uint8_t *buffer, size_t len, int flags) {

	tcp_t *data = sock->data;
	if (data->state != TCP_STATE_OPEN)
		return -ENOTCONN;

	size_t original_len = len;
	while (len && data->state == TCP_STATE_OPEN) {
		if (data->tosend_len == sizeof(data->tosend)) {
			process_sleep(process_get_pid(), 5);
			continue;
		}
		size_t to_copy = TCP_MIN(sizeof(data->tosend) - data->tosend_len, len);
		mem_copy(&data->tosend[data->tosend_len], buffer, to_copy);

		len -= to_copy;
		buffer = &buffer[to_copy];
		data->tosend_len += to_copy;
	}
	return original_len - len;
}
