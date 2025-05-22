/*****************************************************************************\
|   === ethernet.h : 2025 ===                                                 |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef ETHERNET_H
#define ETHERNET_H

#include <ktype.h>

#define IP_PROTO_UDP       17
#define UDP_PORT_DHCP_CLIENT 68
#define UDP_PORT_DHCP_SERVER 67

typedef struct
{
    uint16_t htype; // Hardware type
    uint16_t ptype; // Protocol type
    uint8_t  hlen; // Hardware address length (Ethernet = 6)
    uint8_t  plen; // Protocol address length (IPv4 = 4)
    uint16_t opcode; // ARP Operation Code
    uint8_t  srchw[6]; // Source hardware address - hlen bytes (see above)
    uint8_t  srcpr[4]; // Source protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
    uint8_t  dsthw[6]; // Destination hardware address - hlen bytes (see above)
    uint8_t  dstpr[4]; // Destination protocol address - plen bytes (see above). If IPv4 can just be a "u32" type.
} arp_packet_t;

// En-tête Ethernet (14 octets)
struct eth_header {
    uint8_t  dst_mac[6]; // MAC destination
    uint8_t  src_mac[6]; // MAC source
    uint16_t ethertype;  // 0x0800 pour IPv4
} __attribute__((packed));

typedef struct eth_header eth_header_t;

// En-tête UDP (8 octets)
struct udp_header {
    uint16_t src_port;  // Port source (68)
    uint16_t dst_port;  // Port destination (67)
    uint16_t length;    // Taille totale de l'UDP (header + data)
    uint16_t checksum;  // Checksum UDP (optionnel)
} __attribute__((packed));

typedef struct udp_header udp_header_t;

typedef struct {
    uint8_t *data;
    uint32_t len;
    uint8_t status; // if 0 : ok else bad
} eth_raw_packet_t;

int eth_init(void);

void eth_append_packet(void *data, uint32_t len);
void eth_send_packet(const void *buffer, uint16_t size);

uint32_t eth_get_transactiob_id();

uint16_t htons(uint16_t x);
uint32_t htonl(uint32_t x);

extern uint8_t eth_mac[6];
extern uint8_t eth_ip[4];


#define ETH_MAX_HANDLERS 100

typedef void (*packet_handler_t)(const eth_raw_packet_t *packet);

// return 1 on error, else 0
int eth_register_handler(packet_handler_t handler);
void eth_remove_handlers(packet_handler_t handler);

#endif
