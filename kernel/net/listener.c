#include <net.h>
#include <minilib.h>
#include <kernel/process.h>

/*

struct eth_info_t {
	uint32_t net_mask;
	uint32_t router_ip;
	uint8_t router_mac[6];

	uint32_t ip;
	uint32_t mac;
};

syscalls:
	int eth_start() -> returns a id of the listner (can have multiple by process) (0 == error)
	void eth_end(int id)
	int eth_send(void *data, int len)
	int eth_is_ready(int id) -> -1 if not ready to recv, else returns the size of the next packet
	void eth_recv(int id, void *data)
	void eth_get_info(int id, struct eth_info_t *info)
	void eth_set_info(int id, struct eth_info_t *info)


*/

static eth_listener_t *listeners = NULL;
static int listeners_len = NULL;
static uint32_t last_id = 0;

static void remove_listener_at(int idx) {
	for (int i = 0; i < listeners[idx].len; i++)
		free(listeners[idx].packets[i].data);
	free(listeners[idx].packets);
	for (int i = idx; i < listeners_len - 1; i++)
		listeners[i] = listeners[i + 1];
	listeners_len--;
	listeners = realloc(listeners, sizeof(eth_listener_t) * listeners_len);
}

static eth_listener_t *get_listener(int id, int pid) {
	if (id == 0)
		return NULL;
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].id == id) {
			if (pid < 0 || pid == listeners[i].pid)
				return &listeners[i];
		}
	}
	return NULL;
}

uint32_t eth_start() {
	eth_listener_t *tmp = realloc(listeners, sizeof(eth_listener_t) * (listeners_len + 1));
	if (tmp == NULL)
		return 0;
	listeners = tmp;

	eth_listener_t *lis = &listeners[listeners_len];
	listeners_len++;

	lis->len = 0;
	lis->packets = NULL;
	lis->pid = process_get_pid();
	lis->id = ++last_id;
	if (lis->id == 0)
		lis->id = ++last_id;
	return last_id;
}

void eth_end(uint32_t id) {
	if (id == 0)
		return ;
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].id == id && listeners[i].pid == process_get_pid()) {
			remove_listener_at(i);
			break;
		}
	}
}

int eth_send(void *data, int len) {
	return eth_send_packet(data, (uint16_t)len);
}

int eth_is_ready(uint32_t id) {
	eth_listener_t *lis = get_listener(id, process_get_pid());
	if (lis == NULL)
		return -1;
	if (lis->len == 0)
		return -1;
	return lis->packets[0].len;
}

void eth_recv(uint32_t id, void *data) {
	eth_listener_t *lis = get_listener(id, process_get_pid());
	if (lis == NULL)
		return -1;
	if (lis->len == 0)
		return -1;
	mem_copy(data, lis->packets[0].data, lis->packets[0].len);

	free(lis->packets[0].data);
	for (int i = 0; i < lis->len - 1; i++)
		lis->packets[i] = lis->packets[i + 1];
	lis->len--;
	lis->packets = realloc(lis->packets, sizeof(eth_packet_t) * lis->len);
}

void eth_get_info(uint32_t id, struct eth_info_t *info) {
	(void)id;
	*info = eth_info;
}

void eth_set_info(uint32_t id, struct eth_info_t *info) {
	(void)id;
	eth_info = *info;
}

void eth_listeners_add_packet(const void *addr, int len) {
	// should maybe check if we are in kernel mode if this is in a module
	for (int i = 0; i < listeners_len; i++) {
		eth_listener_t *lis = &listeners[i];

		lis->len++;
		lis->packets = realloc(lis->packets, sizeof(eth_packet_t) * (lis->len));
		lis->packets[lis->len - 1].data = malloc(sizeof(char) * len);
		mem_copy(lis->packets[lis->len - 1].data, addr, len);
		lis->packets[lis->len - 1].len = len;
	}
}