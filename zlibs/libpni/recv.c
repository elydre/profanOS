#include "pni_private.h"
#include <stdarg.h>
#include <unistd.h>

typedef struct {
	int hang_time;
	uint8_t need_port_src;
	uint8_t need_port_dest;
	uint16_t port_src;
	uint16_t port_dest;
} flags_info_t;

static int check_flags(uint32_t flags, flags_info_t *info, va_list *args) {
	if (flags & PNI_RECV_NOHANG && flags & PNI_RECV_HANGTIME)
		return 1;
	if (flags & PNI_RECV_HANGTIME) {
		info->hang_time = va_arg(*args, int);
		if (info->hang_time <= 0)
			return 1;
	}
	if (flags & PNI_RECV_PORT_SRC) {
		info->need_port_src = 1;
		info->port_src = va_arg(*args, uint32_t);
	}
	if (flags & PNI_RECV_PORT_DEST) {
		info->need_port_dest = 1;
		info->port_dest = va_arg(*args, uint32_t);
	}
	return 0;
}

static pni_packet_t parse_packet(uint8_t *raw_packet, int raw_len, flags_info_t info) {
	pni_packet_t packet = {0};
	packet.len = -1;
	if (raw_len < (int)sizeof(ethernet_header_t) + (int)sizeof(ip_header_t) + (int)sizeof(udp_header_t))
		return packet;

	ethernet_header_t *eth = (ethernet_header_t *)raw_packet;
	if (ntohs(eth->ether_type) != 0x0800)
		return packet;


	ip_header_t *ip = (ip_header_t *)(raw_packet + sizeof(ethernet_header_t));
	if (ip->version_ihl != 0x45 || ip->protocol != 17)
		return packet;

	udp_header_t *udp = (udp_header_t *)(raw_packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	int data_len = ntohs(udp->length) - sizeof(udp_header_t);
	if (data_len <= 0 || data_len > raw_len - (int)sizeof(ethernet_header_t) - (int)sizeof(ip_header_t) - (int)sizeof(udp_header_t))
		return packet;
	if (info.need_port_src && ntohs(udp->src_port) != info.port_src)
		return packet;
	if (info.need_port_dest && ntohs(udp->dest_port) != info.port_dest)
		return packet;

	uint8_t *data = malloc(data_len);
	if (!data)
		return packet;
	memcpy(data, raw_packet + sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t), data_len);

	packet.src_port = ntohs(udp->src_port);
	packet.dest_port = ntohs(udp->dest_port);
	memcpy(packet.src_ip, ip->src_ip, sizeof(packet.src_ip));
	packet.len = data_len;
	packet.data = data;
	return packet;
}

pni_packet_t pni_recv(uint32_t flags, ...) {
	if (pni_inited == 0)
		return (pni_packet_t){.len = PNI_ERR_NO_INIT, .data = NULL};

	va_list args;
	flags_info_t info = {0};

	va_start(args, flags);
	if (check_flags(flags, &info, &args))
		return (pni_packet_t){.len = PNI_ERR_FLAGS, .data = NULL};
	va_end(args);

	uint32_t time_end = syscall_timer_get_ms() + info.hang_time;
	while (1) {
		int ready = syscall_eth_listen_isready();
		if (!ready && (flags & PNI_RECV_NOHANG))
			return (pni_packet_t){.len = PNI_ERR_NO_PACKET, .data = NULL};
		if (!ready && (flags & PNI_RECV_HANGTIME)) {
			if (syscall_timer_get_ms() >= time_end)
				return (pni_packet_t){.len = PNI_ERR_NO_PACKET, .data = NULL};
			usleep(500);
			continue;
		}
		if (!ready) {
			usleep(500);
			continue;
		}
		int raw_len = syscall_eth_listen_getsize();
		uint8_t *raw_packet = malloc(raw_len);
		syscall_eth_listen_get(raw_packet);

		pni_packet_t res = parse_packet(raw_packet, raw_len, info);
		free(raw_packet);
		if (res.len == -1)
			continue;

		return res;
	}
}
