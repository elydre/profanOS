/*****************************************************************************\
|   === dhcp.c : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <net/ethernet.h>
#include <net/dhcp.h>
#include <net/ip.h>
#include <minilib.h>
#include <cpu/timer.h>


void build_dhcp_discover(uint8_t *buffer, uint8_t *mac) {
    struct dhcp_header *dhcp = (struct dhcp_header *)buffer;
    mem_set(dhcp, 0, sizeof(struct dhcp_header));

    dhcp->op = 1;  // Request
    dhcp->htype = 1;  // Ethernet
    dhcp->hlen = 6;  // Taille adresse MAC
    dhcp->xid = eth_get_transactiob_id();
    dhcp->flags = htons(0x8000);  // Broadcast
    mem_copy(dhcp->chaddr, mac, 6);  // Adresse MAC du client
    dhcp->magic_cookie = htonl(0x63825363);  // Magic cookie

    // Ajout des options DHCP
    uint8_t *options = dhcp->options;
    *(options)++ = 53; // Option: DHCP Message Type
    *(options)++ = 1;  // Longueur
    *(options)++ = 1;  // DHCPDISCOVER

    *(options)++ = 55; // Option: Parameter Request List
    *(options)++ = 3;  // Longueur
    *(options)++ = 1;  // Subnet Mask
    *(options)++ = 3;  // Router
    *(options)++ = 6;  // DNS Server

    *(options)++ = 255; // End option
}

void build_dhcp_packet(uint8_t *buffer, uint8_t *mac) {
    struct eth_header *eth = (struct eth_header *)buffer;
    struct ip_header *ip = (struct ip_header *)(buffer + sizeof(struct eth_header));
    struct udp_header *udp = (struct udp_header *)(buffer + sizeof(struct eth_header) + sizeof(struct ip_header));
    uint8_t *dhcp = buffer + sizeof(struct eth_header) + sizeof(struct ip_header) + sizeof(struct udp_header);

    // Ethernet
    mem_set(eth->dst_mac, 0xFF, 6);  // Broadcast
    mem_copy(eth->src_mac, mac, 6);
    eth->ethertype = htons(0x0800);

    // IP
    ip->version_ihl = (4 << 4) | 5;
    ip->tos = 0;
    ip->total_length = htons(sizeof(struct ip_header) + sizeof(struct udp_header) + sizeof(struct dhcp_header) + 10);
    ip->id = 0;
    ip->flags_fragment = 0;
    ip->ttl = 64;
    ip->protocol = IP_PROTO_UDP;
    ip->src_ip = 0;  // 0.0.0.0
    ip->dst_ip = htonl(0xFFFFFFFF);  // 255.255.255.255
    ip->checksum = ip_checksum(ip, sizeof(struct ip_header));

    // UDP
    udp->src_port = htons(UDP_PORT_DHCP_CLIENT);
    udp->dst_port = htons(UDP_PORT_DHCP_SERVER);
    udp->length = htons(sizeof(struct udp_header) + sizeof(struct dhcp_header) + 10);
    udp->checksum = 0;  // Pas obligatoire

    // DHCP
	build_dhcp_discover(dhcp, mac);
}
