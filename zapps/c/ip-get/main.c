#include "ip-get.h"

uint32_t last_xid = 0;
uint32_t offered_ip = 0;
uint32_t dhcp_server_ip = 0;

int retrieve_ip(uint8_t *mac) {
	srand(syscall_timer_get_ms());

	send_dhcp_discover(mac);

    uint32_t now = syscall_timer_get_ms();
    int fail = 1;
    while (syscall_timer_get_ms() - now < 1000) {
        fail = receive_offer(mac);
        usleep(500);
    }
    if (fail)
        return 1;
    return 0;
}

int main(int argc, char **argv) {
	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

	uint8_t mac[6];
    uint8_t ip[4] = {0, 0, 0, 0};
    syscall_eth_get_ip(ip);
    if (ip[0] != 0 || ip[1] != 0 || ip[2] != 0 || ip[3] != 0) {
        printf("%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
        return 0;
    }
	syscall_eth_get_mac(mac);
	if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 && mac[3] == 0 && mac[4] == 0 && mac[5] == 0) {
		fprintf(stderr, "No ethernet device has beed inited\n");
		return 1;
	}
	syscall_eth_listen_start();
	int res = retrieve_ip(mac);
    if (res == 0) {
        memcpy(ip, &offered_ip, 4);
        syscall_eth_set_ip(ip);
        printf("%d.%d.%d.%d\n", ip[0], ip[1], ip[2], ip[3]);
    }
	syscall_eth_listen_end();
	return res;
}