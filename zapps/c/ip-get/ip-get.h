#ifndef IP_GET_H
#define IP_GET_H

#include <stdio.h>
#include <stdint.h>
#include <profan/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <profan/net.h>

typedef struct {
	uint8_t dest_mac[6];
	uint8_t src_mac[6];
	uint16_t ether_type;
} __attribute__((packed)) ethernet_header_t;

typedef struct {
	uint8_t version_ihl;
	uint8_t tos;
	uint16_t total_length;
	uint16_t id;
	uint16_t flags_offset;
	uint8_t ttl;
	uint8_t protocol;
	uint16_t checksum;
	uint32_t src_ip;
	uint32_t dest_ip;
} __attribute__((packed)) ip_header_t;

typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__((packed)) udp_header_t;

typedef struct {
	uint8_t op;
	uint8_t htype;
	uint8_t hlen;
	uint8_t hops;
	uint32_t xid;
	uint16_t secs;
	uint16_t flags;
	uint32_t ciaddr;
	uint32_t yiaddr;
	uint32_t siaddr;
	uint32_t giaddr;
	uint8_t chaddr[16];
	uint8_t sname[64];
	uint8_t file[128];
	uint32_t magic_cookie;
} __attribute__((packed)) dhcp_packet_no_opt_t;

extern uint32_t last_xid; //  network endian
extern uint32_t offered_ip;
extern uint32_t dhcp_server_ip;


extern eth_info_t g_info;
extern uint32_t g_eth_id;

void send_dhcp_discover(uint8_t *mac);
int receive_offer(uint8_t *mac);
void send_dhcp_request(uint8_t *mac);
int receive_ack(uint8_t *mac);

#endif