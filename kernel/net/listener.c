#include <net/ethernet.h>
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
	int eth_start() -> returns a id of the listner (can have multiple by process)
	void eth_end(int id)
	int eth_send(void *data, int len)
	int eth_is_ready(int id) -> -1 if not ready to recv, else returns the size of the next packet
	void eth_recv(int id, void *data)
	void eth_get_info(int id, struct eth_info_t *info)
	void eth_set_info(int id, struct eth_info_t *info)


*/



int eth_start(int type) {

}