#ifndef ARP_H
#define ARP_H

#include <ktype.h>

struct arp_packet_v4_t {
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

typedef struct arp_packet_v4_t arp_packet_v4_t;

#endif
