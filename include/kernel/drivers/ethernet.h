#ifndef ETHERNET_H
#define ETHERNET_H

#include <ktype.h>

#define ETHERNET_TYPE_IPV4 0x0800

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

// En-tête DHCP (240 octets sans options)
struct dhcp_header {
    uint8_t  op;       // 1 = Request, 2 = Reply
    uint8_t  htype;    // 1 = Ethernet
    uint8_t  hlen;     // Taille adresse MAC (6)
    uint8_t  hops;     // Nombre de relais (0)
    uint32_t xid;      // Identifiant transaction
    uint16_t secs;     // Secondes écoulées
    uint16_t flags;    // Flags (0x8000 pour broadcast)
    uint32_t ciaddr;   // Client IP (0.0.0.0)
    uint32_t yiaddr;   // IP assignée par le serveur
    uint32_t siaddr;   // IP du serveur
    uint32_t giaddr;   // IP du relais
    uint8_t  chaddr[16]; // Adresse MAC du client (6 octets) + padding
    uint8_t  sname[64]; // Nom du serveur (optionnel)
    uint8_t  file[128]; // Nom du fichier de boot (optionnel)
    uint32_t magic_cookie; // 0x63825363 (Magic DHCP)
    uint8_t  options[]; // Options DHCP
} __attribute__((packed));




// En-tête Ethernet (14 octets)
struct eth_header {
    uint8_t  dst_mac[6]; // MAC destination
    uint8_t  src_mac[6]; // MAC source
    uint16_t ethertype;  // 0x0800 pour IPv4
} __attribute__((packed));

// En-tête IP (20 octets minimum)
struct ip_header {
    uint8_t  ihl:4, version:4; // Longueur et version
    uint8_t  tos;              // Type of Service
    uint16_t total_length;     // Taille totale du paquet
    uint16_t id;               // ID du paquet
    uint16_t flags_fragment;   // Flags et offset de fragmentation
    uint8_t  ttl;              // Time to Live
    uint8_t  protocol;         // UDP = 17
    uint16_t checksum;         // Checksum IP
    uint32_t src_ip;           // IP source (0.0.0.0)
    uint32_t dst_ip;           // IP destination (255.255.255.255)
} __attribute__((packed));

// En-tête UDP (8 octets)
struct udp_header {
    uint16_t src_port;  // Port source (68)
    uint16_t dst_port;  // Port destination (67)
    uint16_t length;    // Taille totale de l'UDP (header + data)
    uint16_t checksum;  // Checksum UDP (optionnel)
} __attribute__((packed));

void eth_send_packet(const void *buffer, uint16_t size);
uint16_t htons(uint16_t x);
uint32_t htonl(uint32_t x);

int eth_init(void);

#endif