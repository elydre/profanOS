#ifndef PACKET_H
#define PACKET_H

#include <ktype.h>

// These packets are not meant to be sen
// they are simplified for reading purposes
// they are a result of parsing the real raw packet
#define ETHER_IPV4 0x0800
#define ETHER_ARP 0x0806

struct eth_arp_v4_t {
	uint16_t htype; // Hardware type 0x01
	uint16_t ptype; // Protocol type 0x0800
	uint8_t hlen; // Hardware len 6
	uint8_t plen; // Protocol len 4
	uint16_t opcode; // ARP Operation Code 1: request, 2: reply
	uint8_t src_haddr[6];
	uint8_t src_paddr[4];
	uint8_t target_haddr[6];
	uint8_t target_paddr[4];
} __attribute__((packed));

typedef struct eth_arp_v4_t eth_arp_v4_t;

struct eth_ip_t {
	uint8_t version;
	uint8_t ihl;
} __attribute__((packed));

typedef struct {
    uint16_t ethertype;
	union {
		eth_arp_v4_t *arp;
		eth_ip_t *ipv4;

	} val;
} eth_playload_t;

typedef struct {
    uint8_t mac_dst[6];
    uint8_t mac_src[6];
    eth_playload_t playload;
} eth_packet_t;


#endif