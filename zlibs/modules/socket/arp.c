#include "arp.h"
#include <minilib.h>

static int respond(arp_packet_t *packet) {
	if (packet->htype != 1 || packet->ptype != 0x0800)
		return 1;
	if (packet->plen != 4 || packet->hlen != 6)
		return 1;
	if (packet->op != 1)
		return 1;

	eth_info_t info;
	eth_get_info(0, &info);
	if (info.ip != packet->t_paddr)
		return 1;
	
	uint8_t response[6 + 6 + 2 + 28];
	mem_copy(response, packet->s_haddr, 6);
	mem_copy(&response[6], info.mac, 6);
	response[12] = 0x08;
	response[13] = 0x06;

	uint8_t *arp = &response[14];
	// htype
	arp[0] = 0;
	arp[1] = 1;

	// ptype
	arp[2] = 0x08;
	arp[3] = 0;

	//hlen/plen
	arp[4] = 6;
	arp[5] = 4;

	//op
	arp[6] = 0;
	arp[7] = 2;

	mem_copy(&arp[8], info.mac, 6);
	mem_copy(&arp[14], &info.ip, 4);
	mem_copy(&arp[18], packet->s_haddr, 6);
	mem_copy(&arp[24], &packet->s_paddr, 4);
	eth_send(response, 6 + 6 + 2 + 28);

    return 0;
}

void socket_on_recv_arp(int len, uint8_t *packet) {
	if (len < 28)
		return ;
	arp_packet_t arp;

	arp.htype = (((uint16_t)packet[0]) << 8) | (packet[1]);
	arp.ptype = (((uint16_t)packet[2]) << 8) | (packet[3]);
	arp.hlen = packet[4];
	arp.plen = packet[5];
	arp.op = (((uint16_t)packet[6]) << 8) | (packet[7]);

	mem_copy(arp.s_haddr, &packet[8], 6);
	mem_copy(&arp.s_paddr, &packet[14], 4);
	mem_copy(arp.t_haddr, &packet[18], 6);
	mem_copy(&arp.t_paddr, &packet[24], 4);
	
	respond(&arp);
}
