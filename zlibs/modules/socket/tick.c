#include "socket.h"
#include "ip.h"
#include "udp.h"

#define ETHER_IP4 0x0800

void socket_tick(int len, uint8_t *packet) {
	for (int i = 0; i < sockets_len; i++) {
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
	if (!len)
		return ;
	if (len < 6 + 6 + 2)
		return ;
	uint16_t ether_type = packet[6 + 6] << 8;
	ether_type |= packet[6 + 6 + 1];
	len -= 6 + 6 + 2;
	packet += 6 + 6 + 2;
	kprintf_serial("ether type %x len %d\n", ether_type, len);
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
