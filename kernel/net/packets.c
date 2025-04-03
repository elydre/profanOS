#include <net/ethernet.h>

static packet_handler_t eth_handlers[ETH_MAX_HANDLERS + 1] = {NULL};

// return 1 on error
int eth_register_handler(packet_handler_t handler) {
	int i = 0;

	while (eth_handlers[i] != NULL)
		i++;
	if (i == ETH_MAX_HANDLERS)
		return 1;
	eth_handlers[i] = handler;
	return 0;
}

void eth_remove_handlers(packet_handler_t handler) {
	int idx = 0;
	int found = 0;

	while (eth_handlers[idx] != NULL) {
		if (eth_handlers[idx] == handler) {
			found = 0;
			break;
		}
		idx++;
	}

	if (found == 0)
		return ;

	for (int i = idx; eth_handlers[i] != NULL; i++)
		eth_handlers[i] = eth_handlers[i + 1];
}

void eth_append_packet(void *data, uint32_t len) {
	eth_raw_packet_t packet = (eth_raw_packet_t){0};
	packet.data = data;
	packet.len = len;


	int i = 0;
	while (eth_handlers[i] != NULL) {
		(*eth_handlers[i])((const eth_raw_packet_t *)&packet);
		i++;
	}
}
