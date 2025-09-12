#include "ip-get.h"


void send_dhcp_discover(uint8_t *mac) {
    uint8_t packet[512];
    memset(packet, 0, sizeof(packet));

    // === Pointeurs vers les structures ===
    ethernet_header_t *eth = (ethernet_header_t *)packet;
    ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
    udp_header_t *udp = (udp_header_t *)((uint8_t *)ip + sizeof(ip_header_t));
    dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)((uint8_t *)udp + sizeof(udp_header_t));
    uint8_t *options = (uint8_t *)(dhcp + 1);

    // === Remplissage Ethernet ===
    memset(eth->dest_mac, 0xFF, 6);  // Broadcast
    memcpy(eth->src_mac, mac, 6);
    eth->ether_type = htons(0x0800); // IPv4

    // === Remplissage IP ===
    ip->version_ihl = 0x45;  // IPv4, IHL = 5
    ip->tos = 0;
    ip->id = htons(0);
    ip->flags_offset = htons(0x4000); // Don't fragment
    ip->ttl = 64;
    ip->protocol = 17; // UDP
    ip->checksum = 0;
    ip->src_ip = htonl(0x00000000); // 0.0.0.0
    ip->dest_ip = htonl(0xFFFFFFFF); // 255.255.255.255

    // === Remplissage UDP ===
    udp->src_port = htons(68);
    udp->dest_port = htons(67);
    udp->checksum = 0; // Calculé plus tard

    // === Remplissage DHCP sans options ===
    memset(dhcp, 0, sizeof(dhcp_packet_no_opt_t));
    dhcp->op = 1; // BOOTREQUEST
    dhcp->htype = 1; // Ethernet
    dhcp->hlen = 6;
    dhcp->xid = htonl(syscall_eth_get_transaction());
	last_xid = dhcp->xid;
    dhcp->flags = htons(0x8000); // Broadcast flag
    memcpy(dhcp->chaddr, mac, 6);
    dhcp->magic_cookie = htonl(0x63825363);

    // === DHCP Options ===
    uint8_t *opt = options;
    *opt++ = 53; *opt++ = 1; *opt++ = 1; // DHCPDISCOVER
    *opt++ = 55; *opt++ = 3; *opt++ = 1; *opt++ = 3; *opt++ = 6; // Parameter request
    *opt++ = 255; // End option

    size_t dhcp_size = sizeof(dhcp_packet_no_opt_t) + (opt - options);
    size_t udp_len = sizeof(udp_header_t) + dhcp_size;
    size_t ip_len = sizeof(ip_header_t) + udp_len;
    size_t total_len = sizeof(ethernet_header_t) + ip_len;

    // === Longueurs ===
    ip->total_length = htons(ip_len);
    udp->length = htons(udp_len);

    // === Checksum IP ===
    ip->checksum = 0;
    uint32_t sum = 0;
    // uint16_t *ip_data = (uint16_t *)ip;
    // for (size_t i = 0; i < sizeof(ip_header_t)/2; ++i)
    //     sum += ntohs(ip_data[i]);
    uint16_t ip_data_buff[sizeof(ip_header_t) / 2];
    memcpy(ip_data_buff, ip, sizeof(ip_header_t));
    for (size_t i = 0; i < sizeof(ip_data_buff) / 2; i++) {
        sum += ntohs(ip_data_buff[i]);
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    ip->checksum = htons(~sum);

    // === Checksum UDP ===
    sum = 0;
    // Pseudo-header
    sum += (ip->src_ip >> 16) & 0xFFFF;
    sum += ip->src_ip & 0xFFFF;
    sum += (ip->dest_ip >> 16) & 0xFFFF;
    sum += ip->dest_ip & 0xFFFF;
    sum += ip->protocol;
    sum += ntohs(udp->length);

    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp;
    for (size_t i = 0; i < udp_len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < udp_len)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    udp->checksum = htons(~sum);

    // === Envoi ===
    syscall_eth_send(packet, total_len);
}
