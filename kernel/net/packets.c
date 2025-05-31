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
	int old_len = listeners_len;

	for (int i = 0; i < listeners_len; i++) {
		listener_t *lis = &listeners[i];
		if (process_wait(lis->pid, NULL, 0)) {
			I_remove_listener(i);
			continue;
		}
		lis->packets = realloc_as_kernel(lis->packets, sizeof(eth_raw_packet_t) * (1 + lis->packets_len));
		lis->packets[i].data = malloc(sizeof(uint8_t) * len);
		lis->packets[i].len = len;
		mem_copy(lis->packets[i].data, data, sizeof(uint8_t) * len);
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

static void I_listener_remove(int pid) {
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
		listener_t *lis = &listeners[i];
		if (lis->packets_len == 0)
			break;
		mem_copy(data, lis->packets[i].data, lis->packets_len * sizeof(uint8_t));
		free(lis->packets[i].data);
		for (int k = 0; k < lis->packets_len - 1; k++)
			lis->packets[i] = lis->packets[i + 1];
		lis->packets_len--;
		break;
	}
}

static listener_t *I_listener_get(int pid) {
	for (int i = 0; i < listeners_len; i++) {
		if (listeners[i].pid == pid)
			return &listeners[i];
	}
	return (NULL);
}



void eth_listen_start() {
	int pid = process_get_pid();

	eth_listener_add(pid);
}

void eth_listen_end() {
	int pid = process_get_pid();

	eth_listener_remove(pid);
}

void eth_listen_get(uint32_t data_ptr) {
	int pid = process_get_pid();

	eth_listener_pop(pid, (void *)data_ptr);
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

void eth_call_send(uint32_t data_ptr, uint32_t len) {
	eth_send_packet((void *)data_ptr, len);
}