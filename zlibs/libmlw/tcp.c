// #include "mlw_private.h"
// #include <sys/time.h>

// struct tcp_recv_info {
// 	uint32_t src_ip;
// 	uint32_t dest_ip;
// 	uint16_t src_port;
// 	uint16_t dest_port;
// 	void *data;
// 	int len;
// 	int id;
// };

// typedef struct tcp_recv_info tcp_recv_info;


// int tcp_get_packet_info(tcp_recv_info *info, uint8_t *packet, int len) { // return 1 if not tcp
// 	packet += 12;
// 	len -= 12;
// 	if (len < 2)
// 		return 1;
// 	uint16_t ether_type = *(uint16_t *)packet;
// 	if (ntohs(ether_type) != 0x0800)
// 		return 1;
// 	len -= 2;
// 	packet += 2;

// 	if (len < sizeof(ip_header_t))
// 		return 1;
// 	ip_header_t *ip_header = (void *)packet;
// 	if (ip_header->protocol != 6)
// 		return 1;
// 	info->src_ip = ip_header->src_ip;
// 	info->dest_ip = ip_header->dest_ip;
// 	if (ip_header->tot_len < len)
// 		len = ip_header->tot_len;
// 	packet += sizeof(ip_header_t);
// 	len -= sizeof(ip_header_t);

// 	if (len < sizeof(tcp_header_t))
// 		return 1;
// 	tcp_header_t *tcp_header = (void *)packet;
// 	info->dest_port = tcp_header->dst_port;
// 	info->src_port = tcp_header->src_port;
// 	return 0;
// }

// uint16_t tcp_checksum(uint32_t src_ip, uint32_t dest_ip,
// 						const uint8_t *tcp_seg, int tcp_len) {
// 	pseudo_header_t psh;
// 	psh.src_ip = src_ip;
// 	psh.dest_ip = dest_ip;
// 	psh.zero = 0;
// 	psh.protocol = 6;
// 	psh.tcp_len = htons(tcp_len);

// 	int total_len = sizeof(psh) + tcp_len;
// 	uint8_t *buf = malloc(total_len);
// 	memcpy(buf, &psh, sizeof(psh));
// 	memcpy(buf + sizeof(psh), tcp_seg, tcp_len);
// 	uint32_t sum = 0;
// 	uint16_t *w = (uint16_t *)buf;
// 	for (int i = 0; i < total_len/2; i++) {
// 	    sum += htons(w[i]);
// 		if (sum >> 16)
// 			sum = (sum & 0xFFFF) + (sum >> 16);
// 	}
// 	return ~htons(sum) & 0xFFFF;
// }

// static int mlw_read_buffer(void *buffer, int buffer_len, mlw_instance_t *info) {
// 	int min_len = buffer_len;
// 	if (info->buffer_len < min_len)
// 		min_len = info->buffer_len;

// 	memcpy(buffer, info->buffer, min_len);
// 	memmove(info->buffer, &info->buffer[min_len], info->buffer_len - min_len);
// 	info->buffer_len -= min_len;
// 	info->buffer = realloc(info->buffer, info->buffer_len);
// 	return min_len;
// }

// static void mlw_update_tcp(mlw_instance_t *info) {
// 	while (1) {
// 		if (!info->packets)
// 			return ;
// 		int size = syscall_eth_is_ready(info->eth_id);
// 		if (size <= 0)
// 			return ;
// 		uint8_t *packet = malloc(size);
// 		syscall_eth_recv(info->eth_id, packet);

// 		tcp_recv_info recv_info;
// 		if (tcp_get_packet_info(&recv_info, packet, size)) {
// 			free(packet);
// 			continue;
// 		}
// 		free(packet);

// 	}
// }

// static uint32_t get_time() {
// 	struct timeval t;

// 	gettimeofday(&t, NULL);
// 	return t.tv_sec * 1000 + t.tv_usec / 1000;
// }

// int mlw_tcp_recv(mlw_instance_t *info, void *buffer, int buffer_len, int timeout) { // returns how much have been read
// 	if (buffer_len == 0 || !buffer || !info)
// 		return 0;
// 	if (info->buffer_len)
// 		return mlw_read_buffer(buffer, buffer_len, info);

// 	uint32_t end = get_time() + timeout;
// 	while (1) {
// 		mlw_update_tcp(info);

// 		if (info->buffer_len)
// 			return mlw_read_buffer(buffer, buffer_len, info);

// 		if (timeout < 0)
// 			continue;
// 		if (get_time() >= end)
// 			break;
// 	}
// 	return 0;
// }
