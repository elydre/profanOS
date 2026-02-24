#include "udp.h"
#include <sys/socket.h>
#include <errno.h>
#include "ip.h"

int socket_init_udp(socket_t *sock) {
	udp_t *info = malloc(sizeof(udp_t));
	if (!info)
		return 1;
	sock->data = info;

	info->local_ip = 0;
	info->local_port = 0;
	info->is_bound = 0;
	info->remote_ip = 0;
	info->remote_port = 0;
	info->is_connected = 0;
	info->recv_len = 0;
	info->send_len = 0;
	
	return 0;
}

void socket_on_recv_udp(uint32_t src_ip, uint32_t dest_ip, uint8_t *data, int data_len) {
	if (data_len < 8)
		return ;

	udp_packet_t packet;
	packet.src_ip = src_ip;
	packet.dest_ip = dest_ip;
	packet.len = data_len - 8;
	packet.data = data + 8;

	packet.src_port = data[0] << 8;
	packet.src_port |= data[1];

	packet.dest_port = data[2] << 8;
	packet.dest_port |= data[3];

	int len2 = ((data[4] << 8) | data[5]) - 8;
	kprintf_serial("udp: len %d real len %d\n", len2, packet.len);
	if (len2 > packet.len)
		return ;
	packet.len = len2;
	kprintf_serial("packet received si %x di %x sp %d dp %d\n", packet.src_ip, packet.dest_ip, packet.src_port, packet.dest_port);
}

uint16_t ntohs(uint16_t x) {
	return (x >> 8) | (x << 8);
}

uint32_t ntohl(uint32_t x) {
	uint32_t res = x >> 24;

	res |= ((x >> 16) & 0xff) << 8;
	res |= ((x >> 8) & 0xff) << 16;
	res |= (x & 0xff) << 24;
	return res;
}

#define htons ntohs
#define htonl ntohl

static uint16_t udp_checksum(void *data, int len, uint32_t src_ip, uint32_t dest_ip) {
	uint8_t *udp_data = (uint8_t *)data;
	uint32_t sum = 0;
	uint8_t buffer[12];
	mem_copy(buffer, &src_ip, 4);
	mem_copy(buffer + 4, &dest_ip, 4);
	buffer[8] = 0;
	buffer[9] = 17;
	buffer[10] = len >> 8;
	buffer[11] = len & 0xff;
	for (int i = 0; i < 12; i += 2) {
		uint16_t word = buffer[i] << 8;
		word |= buffer[i + 1];
		sum += word;
		sum = (sum & 0xFFFF) + (sum >> 16);
	}

	for (int i = 0; i < len; i += 2) {
		uint16_t word = udp_data[i] << 8;
		if (i + 1 < len)
			word |= udp_data[i + 1];
		sum += word;
		sum = (sum & 0xFFFF) + (sum >> 16);
	}
	sum = (sum & 0xFFFF) + (sum >> 16);
	sum += (sum >> 16);
	kprintf("%x checksum\n", ~sum);
	return ~sum;
}

#define IP_OFFSET (6 + 6 + 2)
#define UDP_OFFSET (IP_OFFSET + 20)

static void fill_udp(uint8_t *buffer, udp_packet_t *packet) {
	buffer[0] = packet->src_port & 0xff;
	buffer[1] = packet->src_port >> 8;
	buffer[2] = packet->dest_port & 0xff;
	buffer[3] = packet->dest_port >> 8;

	int len = packet->len + 8;
	buffer[4] = len >> 8;
	buffer[5] = len & 0xff;
	buffer[6] = 0;
	buffer[7] = 0;
	mem_copy(&buffer[8], packet->data, len - 8);
	uint16_t checksum = udp_checksum(buffer, len, packet->src_ip, packet->dest_ip);
	buffer[6] = checksum >> 8;
	buffer[7] = checksum & 0xff;
}

static uint16_t ip_checksum(void *buf, int len) {
	uint32_t sum = 0;
	uint16_t *w = buf;
	for (int i = 0; i < len / 2; i++) {
		sum += ntohs(w[i]);
		if (sum > 0xFFFF)
			sum = (sum & 0xFFFF) + (sum >> 16);
	}
	return ~sum & 0xFFFF;
}

static void fill_ip(uint8_t *buffer, udp_packet_t *packet) {
	buffer[0] = 0x45;
	buffer[1] = 0;
	int total_len = packet->len + 8 + 20;
	buffer[2] = total_len >> 8;
	buffer[3] = total_len & 0xff;
	buffer[4] = 0;
	buffer[5] = 0;
	buffer[6] = 0x40;
	buffer[7] = 0x00;
	buffer[8] = 64;
	buffer[9] = 17;
	buffer[10] = 0;
	buffer[11] = 0;
	mem_copy(&buffer[12], &packet->src_ip, 4);
	mem_copy(&buffer[16], &packet->dest_ip, 4);
	uint16_t checksum = ip_checksum(buffer, 20);
	buffer[10] = checksum >> 8;
	buffer[11] = checksum & 0xff;
}

void socket_udp_tick(socket_t *sock) {
	udp_t *info = sock->data;
	if (!info->send_len)
		return ;
	info->send_len--;
	static uint8_t packet[2048]; // do not destroy the stack
	fill_udp(packet + UDP_OFFSET, &info->send[info->send_len]);
	fill_ip(packet + IP_OFFSET, &info->send[info->send_len]);
	mem_copy(packet, &eth_info.router_mac, 6);
	mem_copy(packet + 6, &eth_info.mac, 6);
	packet[12] = 0x08;
	packet[13] = 0x00;
	eth_send(packet, 6 + 6 + 2 + 20 + 8 + info->send[info->send_len].len);
}

int socket_udp_port_is_free(uint16_t port) {
	for (int i = 0; i < sockets_len; i++) {
		if (sockets[i].type != SOCKET_UDP)
			continue;
		udp_t *udp = sockets[i].data;
		if (udp->local_port == port)
			return 1;
	}
	return 0;
}

uint16_t socket_udp_get_free_port() {
	uint16_t res = CLT_PORT_START;
	while (res <= CLT_PORT_END) {
		if (!socket_udp_port_is_free(res))
			res++;
		return res;
	}
	return 0;
}

int socket_udp_bind(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
	udp_t *data = sock->data;
	if (addrlen != sizeof(struct sockaddr_in))
		return -EINVAL;
	if (data->is_bound)
		return -EINVAL;
	const struct sockaddr_in *addr2 = (void *)addr;
	if (addr2->sin_family != AF_INET)
		return -EINVAL;
	uint16_t port = addr2->sin_port;
	if (port && !socket_udp_port_is_free(port))
		return -EADDRINUSE;
	if (port == 0)
		port = socket_udp_get_free_port();
	if (port == 0)
		return -ENOMEM;
	data->local_ip = addr2->sin_addr.s_addr;
	data->local_port = port;
	data->is_bound = 1;
	return 0;
}

int socket_udp_connect(socket_t *sock, const struct sockaddr *addr, socklen_t addrlen) {
	udp_t *data = sock->data;
	if (addrlen != sizeof(struct sockaddr_in))
		return -EINVAL;
	const struct sockaddr_in *addr2 = (void *)addr;
	if (addr2->sin_family != AF_INET)
		return -EINVAL;
	uint16_t port = addr2->sin_port;
	if (port == 0)
		return -EINVAL;
	if (!data->is_bound) {
		struct sockaddr_in addr;
		addr.sin_family = AF_INET;
		addr.sin_addr.s_addr = htonl(INADDR_ANY);
		addr.sin_port = 0;
		
		int err = socket_udp_bind(sock, (void *)&addr, sizeof(addr));
		if (err)
			return err;
	}
	data->is_connected = 1;
	data->remote_port = port;
	data->remote_ip = addr2->sin_addr.s_addr;
	return 0;
}

ssize_t socket_udp_send(socket_t *sock, const uint8_t *buffer, size_t len, uint32_t dip, uint16_t dport) {
	udp_t *data = sock->data;
	if (len > 1450)
		return -EMSGSIZE;
	if (data->send_len == 64)
		return -ENOMEM;
	if (dip == 0)
		dip = data->remote_ip;
	if (dport)
		dport = data->remote_port;
	udp_packet_t *pakcet = data->send[data->send_len++];
	pakcet->src_ip = data->local_ip; // if 0 choose with eth_info
	return len;
}

ssize_t socket_udp_sendto(socket_t *sock, const void *buf, size_t len, int flags, const struct sockaddr *dest_addr, socklen_t addrlen) {
	if (!dest_addr && addrlen == 0)
		return socket_udp_send(sock, buf, len, 0, 0);
}
