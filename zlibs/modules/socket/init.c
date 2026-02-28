#include <modules/socket.h>
#include "udp.h"

static void socket_process() {

	uint32_t eth_id = eth_start();
	uint8_t *packet = malloc(0xFFFF);
	int alloc_len = 0xFFFF;
	while (1) {
		int packet_len = eth_is_ready(eth_id);
		if (packet_len > alloc_len) {
			free(packet);
			alloc_len = packet_len;
			packet = malloc(alloc_len);
		}
		if (packet_len >= 0)
			eth_recv(eth_id, packet);
		if (packet_len < 0)
			packet_len = 0;

		if (packet_len)
			socket_tick(packet_len, packet);
		else
			socket_tick(0, NULL);
		if (!packet)
			process_sleep(process_get_pid(), 1);
	}
}


int __init() {
	process_wakeup(process_create(socket_process, 0, 0, NULL), 0);
	return 0;
}
