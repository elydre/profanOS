#include "ip-get.h"

uint32_t last_xid = 0;
uint32_t offered_ip = 0;
uint32_t dhcp_server_ip = 0;

eth_info_t g_info;
uint32_t g_eth_id;

int retrieve_ip(uint8_t *mac) {
	srand(syscall_timer_get_ms());

	send_dhcp_discover(mac);

    uint32_t now = syscall_timer_get_ms();
    int fail = 1;
    while (syscall_timer_get_ms() - now < 1000) {
        fail = receive_offer(mac);
        if (fail == 0)
            break;
        usleep(500);
    }
    if (fail)
        return 1;
    send_dhcp_request(mac);
    now = syscall_timer_get_ms();
    fail = 1;
    while (syscall_timer_get_ms() - now < 1000) {
        fail = receive_ack(mac);
        if (fail == 0)
            break;
        usleep(500);
    }
    return fail;
}

void print_ip(uint32_t ip) {
    printf("%d.%d.%d.%d\n", ip >> 24, 0xff & (ip >> 16), 0xff & (ip >> 8), ip & 0xff);

}

int main(int argc, char **argv) {
	if (argc > 1) {
		fprintf(stderr, "Usage: %s\n", argv[0]);
		return 1;
	}

    g_eth_id = syscall_eth_start();
    if (g_eth_id == 0) {
        fprintf(stderr, "Error: could not open connection wiith profan net\n");
        return 1;
    }

    syscall_eth_get_info(g_eth_id, &g_info);
    if (g_info.ip != 0) {
        print_ip(g_info.ip);
        syscall_eth_end(g_eth_id);
        return 0;

    }
    if (!memcmp(g_info.mac, "\0\0\0\0\0\0", 6)) {
        fprintf(stderr, "Error: no ethernet device found\n");
        syscall_eth_end(g_eth_id);
        return 1;
    }
    int res = retrieve_ip(g_info.mac);
    if (res == 0) {
        memcpy(&g_info.ip, &offered_ip, 4);
        syscall_eth_set_info(g_eth_id, &g_info);
        print_ip(offered_ip);
    }

    syscall_eth_end(g_eth_id);
    return res;
}