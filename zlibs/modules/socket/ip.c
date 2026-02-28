#include "ip.h"
#include "udp.h"

void socket_on_recv_ip4(int len, uint8_t *packet) {
	if (len < 20)
		return ;

	ip_header_t header;	
	header.v_ihl = packet[0];
	header.tos = packet[1];

	header.tot_len = packet[2] << 8;
	header.tot_len |= packet[3];
	if (header.tot_len > len)
		return ;

	header.id = packet[4] << 8;
	header.id |= packet[5];

	header.flags_n_offset = packet[6] << 8;
	header.flags_n_offset |= packet[7];

	header.ttl = packet[8];
	header.protocol = packet[9];

	header.checksum = packet[10] << 8;
	header.checksum = packet[11];

	mem_copy(&header.src_ip, &packet[12], 4);
	mem_copy(&header.dest_ip, &packet[16], 4);

	header.options = &packet[20];
	header.options_len = (header.v_ihl & 0x0f) * 4 - 20;
	if (header.options_len == 0)
		header.options = NULL;
	header.data_len = header.tot_len - 20 - header.options_len;
	header.data = packet + 20 + header.options_len;

	switch (header.protocol) {
		case 17: // udp
			socket_on_recv_udp(header.src_ip, header.dest_ip, header.data, header.data_len);
			break;
		default:
			break;
	}
}

void socket_ip_send(uint32_t src_ip, uint32_t dest_ip, uint8_t protocol, uint8_t *data, int data_len) {
	
	static uint8_t packet[2048];
}
