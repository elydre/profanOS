/*****************************************************************************\
|   === pni_private.h : 2025 ===                                              |
|                                                                             |
|    Profan Network Interface - simple udp communcation            .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef PNI_PRIVATE_H
#define PNI_PRIVATE_H

#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <profan/syscall.h>
#include <profan/pni.h>

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
	uint8_t src_ip[4];
	uint8_t dest_ip[4];
} __attribute__((packed)) ip_header_t;

typedef struct {
	uint16_t src_port;
	uint16_t dest_port;
	uint16_t length;
	uint16_t checksum;
} __attribute__((packed)) udp_header_t;

extern uint8_t router_mac[6];
extern uint8_t self_ip[4];
extern uint8_t self_mac[6];
extern int pni_inited;

#endif
