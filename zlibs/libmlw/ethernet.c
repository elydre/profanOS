#include "mlw.h"

int mlw_send_ethernet(uint16_t type_lit, uint8_t *mac, uint8_t *data, int len) {
	uint8_t *packet = malloc(6 + 6 + 2 + len);
	if (!packet)
		return 1;
	memcpy(packet, mac, 6);
	memcpy(&packet[6], mlw_eth_info->mac, 6);
	*(uint16_t *)(void *)&packet[12] = htons(type_lit);
	memcpy(&packet[14], data, len);
	int ret = syscall_eth_send(packet, len + 6 + 6 + 2);
	free(packet);
	return ret;
}
