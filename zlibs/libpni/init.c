#include "pni_private.h"
#include <stdio.h>

uint8_t router_mac[6] = {0};
uint8_t self_ip[4] = {0};
uint8_t self_mac[6] = {0};
int pni_inited = 0;

int pni_init() {
	syscall_eth_get_mac(self_mac);
	if (self_mac[0] == 0 && self_mac[1] == 0 && self_mac[2] == 0 && self_mac[3] == 0
		&& self_mac[4] == 0 && self_mac[5] == 0) {
		return PNI_ERR_NO_MAC;
	}

	syscall_eth_get_ip(self_ip);
	printf("SELF ip %d.%d.%d.%d\n", self_ip[0], self_ip[1], self_ip[2], self_ip[3]);
	if (self_ip[0] == 0 && self_ip[1] == 0 && self_ip[2] == 0 && self_ip[3] == 0) {
		return PNI_ERR_NO_IP;
	}

	FILE *f = fopen("/zada/router_mac", "rb");
	if (f == NULL)
		return PNI_ERR_NO_ROUTER;
	fread(router_mac, 6, 1, f);
	fclose(f);

	if (router_mac[0] == 0 && router_mac[1] == 0 && router_mac[2] == 0 && router_mac[3] == 0
		&& router_mac[4] == 0 && router_mac[5] == 0) {
		return PNI_ERR_NO_MAC;
	}

	syscall_eth_listen_start();
	pni_inited = 1;
	return 0;
}
