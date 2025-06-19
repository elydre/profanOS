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
		*len = syscall_eth_listen_getsize();
		uint8_t *packet = malloc(*len);
		syscall_eth_listen_get(packet);
		return packet;
	}
	return NULL;
}
