#include <net/ethernet.h>
#include <minilib.h>
#include <kernel/process.h>

#define LISTENERS_MAX_PACKET 255

typedef struct {
	int pid;
	eth_raw_packet_t *packets;
	int packets_len;
} listener_t;

static listener_t *listeners = NULL;
static int listeners_len = 0;

static void I_remove_listener(int idx) {
	for (int i = 0; i < listeners[idx].packets_len; i++)
		free(listeners[idx].packets[i].data);
	free(listeners[idx].packets);
	for (int i = idx; i < listeners_len - 1; i++)
		listeners[i] = listeners[i + 1];
	listeners_len--;
}

void eth_append_packet(void *data, uint32_t len) {
	if (len < 12)
		return ;
	uint8_t mac[6];
	eth_get_mac(mac);
	// if dest != mac && dest != broadcast return
	if (mem_cmp(data, mac, 6) != 0 && mem_cmp(data, "\xFF\xFF\xFF\xFF\xFF\xFF", 6) != 0)
		return;
	int old_len = listeners_len;

	for (int i = 0; i < listeners_len; i++) {
		listener_t *lis = &listeners[i];
		if (process_is_dead(lis->pid)) {
			I_remove_listener(i);
			continue;
		}
		kprintf_serial("    to pid %d\n", lis->pid);
		lis->packets = realloc_as_kernel(lis->packets, sizeof(eth_raw_packet_t) * (1 + lis->packets_len));
		lis->packets[i].data = malloc(sizeof(uint8_t) * len);
		lis->packets[i].len = len;
		mem_copy(lis->packets[i].data, data, sizeof(uint8_t) * len);
		lis->packets_len++;
	}
	if (old_len != listeners_len)
		listeners = realloc_as_kernel(listeners, sizeof(listener_t) * listeners_len);
}

static void I_listener_add(int pid) {
	listeners = realloc_as_kernel(listeners, sizeof(listener_t) * (listeners_len + 1));
	listeners[listeners_len].pid = pid;
	listeners[listeners_len].packets_len = 0;
	listeners[listeners_len].packets = NULL;
	listeners_len++;
}

// not static for process_kill
void I_listener_remove(int pid) {
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].pid == pid) {
			I_remove_listener(i);
			break;
		}
	}
}

static void I_listener_pop(int pid, void *data) {
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].pid != pid)
			continue;
		if (listeners[i].packets_len == 0)
			break;
		eth_raw_packet_t *packet = &listeners[i].packets[0];
		mem_copy(data, packet->data, packet->len);
		free(packet->data);
		listeners[i].packets_len--;
		if (listeners[i].packets_len == 0) {
			free(listeners[i].packets);
			listeners[i].packets = NULL;
		} else {
			for (int j = 0; j < listeners[i].packets_len; j++)
				listeners[i].packets[j] = listeners[i].packets[j + 1];
			listeners[i].packets = realloc_as_kernel(listeners[i].packets, sizeof(eth_raw_packet_t) * listeners[i].packets_len);
		}
	}
}

static listener_t *I_listener_get(int pid) {
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].pid == pid)
			return &listeners[i];
	}
	return (NULL);
}



int eth_listen_start() {
	int pid = process_get_pid();

	I_listener_add(pid);
	return 0;
}

int eth_listen_end() {
	int pid = process_get_pid();

	I_listener_remove(pid);
	return 0;
}

int eth_listen_get(void *data_ptr) {
	int pid = process_get_pid();

	I_listener_pop(pid, (void *)data_ptr);
	return 0;
}

uint32_t eth_listen_getsize() {
	int pid = process_get_pid();
	listener_t *lis = I_listener_get(pid);

	if (lis == NULL)
		return 0;
	if (lis->packets_len == 0)
		return 0;
	return lis->packets[0].len;

}

uint32_t eth_listen_isready() {
	int pid = process_get_pid();
	listener_t *lis = I_listener_get(pid);

	if (lis == NULL)
		return 0;
	if (lis->packets_len == 0)
		return 0;
	return 1;
}

int eth_send(void *data_ptr, uint32_t len) {
	uint32_t addr = (uint32_t)data_ptr;
	addr -= addr % 4096;
	addr = (uint32_t)scuba_call_phys((void *)addr);
	addr += (uint32_t)data_ptr % 4096;
	eth_send_packet((const void *)addr, len);
	return 0;
}

int eth_get_mac(uint8_t *mac) {
	mac[0] = eth_mac[0];
	mac[1] = eth_mac[1];
	mac[2] = eth_mac[2];
	mac[3] = eth_mac[3];
	mac[4] = eth_mac[4];
	mac[5] = eth_mac[5];
	return 0;
}

uint32_t eth_get_transaction_id() {
	static uint32_t id = 0xABCD0000;
	return id++;
}

int eth_set_ip(uint8_t *ip) {
	eth_ip[0] = ip[0];
	eth_ip[1] = ip[1];
	eth_ip[2] = ip[2];
	eth_ip[3] = ip[3];
	return 0;
}

int eth_get_ip(uint8_t *ip) {
	ip[0] = eth_ip[0];
	ip[1] = eth_ip[1];
	ip[2] = eth_ip[2];
	ip[3] = eth_ip[3];
	return 0;
}
