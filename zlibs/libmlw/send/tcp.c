#include "../mlw_private.h"
#include <unistd.h>

uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip,
						const uint8_t *tcp_seg, int tcp_len) {
	pseudo_header_t psh;
	psh.src_ip = src_ip;
	psh.dest_ip = dest_ip;
	psh.zero = 0;
	psh.protocol = 6;
	psh.tcp_len = htons(tcp_len);

	int total_len = sizeof(psh) + tcp_len;
	uint8_t *buf = malloc(total_len);
	memcpy(buf, &psh, sizeof(psh));
	memcpy(buf + sizeof(psh), tcp_seg, tcp_len);
	uint32_t sum = 0;
	uint16_t *w = (uint16_t *)buf;
	for (int i = 0; i < total_len/2; i++) {
	    sum += htons(w[i]);
		if (sum >> 16)
			sum = (sum & 0xFFFF) + (sum >> 16);
	}
    if (total_len % 2) {
        sum += htons((uint16_t)buf[total_len - 1]);
        if (sum >> 16)
			sum = (sum & 0xFFFF) + (sum >> 16);
    }
	return ~htons(sum) & 0xFFFF;
}


int mlw_tcp_general_send(uint16_t src_port,
				uint32_t dest_ip, uint32_t dest_port, uint32_t seq,
				uint32_t ack, uint8_t flags, uint8_t *data, int len, uint16_t window) {

    int tcp_len = sizeof(tcp_header_t) + len;

	uint32_t src_ip = mlw_info->ip;
	uint8_t *packet = malloc(tcp_len);
	if (!packet)
		return 1;

	tcp_header_t *tcp = (tcp_header_t *)(void *)packet;
	memset(tcp, 0, sizeof(tcp_header_t));
	tcp->src_port = htons(src_port);
	tcp->dst_port = htons(dest_port);
	tcp->seq_num = htonl(seq);
	tcp->ack_num = htonl(ack);
	tcp->data_offset = (5 << 4);
	tcp->flags = flags;
	tcp->window = htons(window);
	tcp->urgent_ptr = 0;

	if (data && len > 0)
		memcpy(packet + sizeof(tcp_header_t), data, len);
    	tcp->checksum = 0;
	tcp->checksum = tcp_checksum(src_ip, dest_ip, packet, tcp_len);

	int ret = mlw_send_ip(6, dest_ip, packet, tcp_len);
	free(packet);
	return ret;
}

int mlw_tcp_send(mlw_instance_t *inst, void *data, int len) {
    if (!inst || !inst->is_open)
        return 1;

    const int MAX_SEGMENT = 500;

    if (len > MAX_SEGMENT) {
        uint8_t *data_ptr = data;
        while (len > 0) {
            int chunk = (len > MAX_SEGMENT) ? MAX_SEGMENT : len;
            if (mlw_tcp_send(inst, data_ptr, chunk))
                return 1;
            data_ptr += chunk;
            len -= chunk;
        }
        return 0;
    }

    int tries = 5;
    while (tries--) {

    }
    return 0;
}
