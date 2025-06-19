#include "pni_private.h"
#include <stdarg.h>
#include <unistd.h>

typedef struct {
	int hang_time;
} flags_info_t;

static int check_flags(uint32_t flags, flags_info_t *info, va_list *args) {
	if (flags & PNI_RECV_NOHANG && flags & PNI_RECV_HANGTIME)
		return 1;
	if (flags & PNI_RECV_HANGTIME) {
		info->hang_time = va_arg(*args, int);
		if (info->hang_time <= 0)
			return 1;
	}
	return 0;
}

static uint8_t *parse_packet(uint8_t *raw_packet, int raw_len, int *len) {
	if (raw_len < (int)sizeof(ethernet_header_t) + (int)sizeof(ip_header_t) + (int)sizeof(udp_header_t))
		return NULL;

	ethernet_header_t *eth = (ethernet_header_t *)raw_packet;
	if (ntohs(eth->ether_type) != 0x0800)
		return NULL;

	ip_header_t *ip = (ip_header_t *)(raw_packet + sizeof(ethernet_header_t));
	if (ip->version_ihl != 0x45 || ip->protocol != 17)
		return NULL;

	udp_header_t *udp = (udp_header_t *)(raw_packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	int data_len = ntohs(udp->length) - sizeof(udp_header_t);
	if (data_len <= 0 || data_len > raw_len - (int)sizeof(ethernet_header_t) - (int)sizeof(ip_header_t) - (int)sizeof(udp_header_t))
		return NULL;
	if (data_len > PNI_MAX_PACKET)
		return NULL;

	uint8_t *data = malloc(data_len);
	if (!data)
		return NULL;
	memcpy(data, raw_packet + sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t), data_len);

	*len = data_len;
	return data;
}

uint8_t *pni_recv(int *len, uint32_t flags, ...) {
	va_list args;
	va_start(args, flags);
	flags_info_t info = {0};
	if (check_flags(flags, &info, &args)) {
		*len = PNI_ERR_FLAGS;
		return NULL;
	}
	va_end(args);
	if (pni_inited == 0) {
		*len = PNI_ERR_NO_INIT;
		return NULL;
	}
	uint32_t time_end = syscall_timer_get_ms() + info.hang_time;
	while (1) {
		int ready = syscall_eth_listen_isready();
		if (!ready && (flags & PNI_RECV_NOHANG)) {
			*len = PNI_ERR_NO_PACKET;
			return NULL;
		}
		if (!ready && (flags & PNI_RECV_HANGTIME)) {
			if (syscall_timer_get_ms() >= time_end) {
				*len = PNI_ERR_NO_PACKET;
				return NULL;
			}
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

		uint8_t *res = parse_packet(raw_packet, raw_len, len);
		free(raw_packet);
		if (res == NULL)
			continue;
		return res;
	}
	return NULL;
}
