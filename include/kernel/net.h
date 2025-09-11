#ifndef NET_H
#define NET_H

typedef struct eth_info_t {
	uint32_t net_mask;
	uint32_t router_ip;
	uint8_t router_mac[6];
	uint8_t pad0[2];

	uint32_t ip;
	uint8_t mac[6];
	uint8_t pad1[2];
} eth_info_t;

typedef struct {
	int len;
	uint8_t *data;
} eth_packet_t;

typedef struct eth_listener_t {
	uint8_t type;
	int pid;
	int len;
	eth_packet_t *packets;
} eth_listener_t;

extern eth_info_t eth_info;

#endif
