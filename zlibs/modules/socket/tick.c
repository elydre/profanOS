#include <modules/socket.h>
#include "ip.h"
#include "udp.h"

#define ETHER_IP4 0x0800
#define is_dead(pid) (process_info((pid), PROC_INFO_STATE, NULL) < 2)

void socket_tick(int len, uint8_t *packet) {
	for (int i = 0; i < sockets_len; i++) {
		if (is_dead(sockets[i].parent_pid))
			sockets[i].parent_pid = -1;
		switch (sockets[i].type) {
			case SOCKET_UDP:
				socket_udp_tick(&sockets[i]);
				break;
			default:
				sys_warning("%d %d %d\n", AF_INET, SOCK_DGRAM, 0);
				sys_warning("Invalid socket type %d %d %d\n", 
					sockets[i].type & 0xff,
					(sockets[i].type >> 8) & 0xff,
					(sockets[i].type >> 16));
				break;
		}
	}
	for (int i = 0; i < sockets_len; i++) {
		if (sockets[i].do_remove) {
			mem_copy(&sockets[i], &sockets[i + 1], sockets_len - i - 1);
			sockets_len--;
			i--;
		}
	}
	if (!len)
		return ;
	if (len < 6 + 6 + 2)
		return ;
	uint16_t ether_type = packet[6 + 6] << 8;
	ether_type |= packet[6 + 6 + 1];
	len -= 6 + 6 + 2;
	packet += 6 + 6 + 2;
	if (!len)
		return ;
	switch (ether_type) {
		case ETHER_IP4:
			socket_on_recv_ip4(len, packet);
			break;
		default:
			break;
	}
}
