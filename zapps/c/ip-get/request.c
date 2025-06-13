#include "ip-get.h"

void send_dhcp_request(uint8_t *mac) {
	int total_size = sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t)
		+ sizeof(dhcp_packet_no_opt_t) + 16;
	uint8_t *packet = malloc(total_size);
	if (!packet)
		return;

	ethernet_header_t *eth = (ethernet_header_t *)packet;
	ip_header_t *ip = (ip_header_t *)(packet + sizeof(ethernet_header_t));
	udp_header_t *udp = (udp_header_t *)(packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	dhcp_packet_no_opt_t *dhcp = (dhcp_packet_no_opt_t *)(packet + sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t));
	uint8_t *opt = (uint8_t *)(dhcp + 1);

	memcpy(eth->dest_mac, "\xFF\xFF\xFF\xFF\xFF\xFF", 6);
	memcpy(eth->src_mac, mac, 6);
	eth->ether_type = htons(0x0800);

	ip->version_ihl = 0x45;
    ip->tos = 0;
    ip->id = htons(0);
    ip->flags_offset = htons(0x4000);
    ip->ttl = 64;
    ip->protocol = 17; // UDP
    ip->checksum = 0;
    ip->src_ip = htonl(0x00000000); // 0.0.0.0
    ip->dest_ip = htonl(0xFFFFFFFF); // 255.255.255.255
	ip->total_length = htons(sizeof(ip_header_t) + sizeof(udp_header_t) + sizeof(dhcp_packet_no_opt_t) + 16);

	udp->src_port = htons(68);
	udp->dest_port = htons(67);
	udp->length = htons(sizeof(udp_header_t) + sizeof(dhcp_packet_no_opt_t) + 16);
	udp->checksum = 0;

	memset(dhcp, 0, sizeof(dhcp_packet_no_opt_t));
	dhcp->op = 1; // BOOTREQUEST
	dhcp->htype = 1; // Ethernet
	dhcp->hlen = 6;
	dhcp->hops = 0;
	dhcp->xid = last_xid;
	dhcp->secs = htons(1);
	dhcp->flags = htons(0x8000);
	dhcp->ciaddr = 0;
	dhcp->yiaddr = 0;
	dhcp->siaddr = dhcp_server_ip;
	dhcp->giaddr = 0;
	memcpy(dhcp->chaddr, mac, 6);
	memset(dhcp->sname, 0, sizeof(dhcp->sname));
	memset(dhcp->file, 0, sizeof(dhcp->file));
	dhcp->magic_cookie = htonl(0x63825363);
	*(opt++) = 53; // DHCP Message Type
	*(opt++) = 1; // Length
	*(opt++) = 3; // DHCPREQUEST
	*(opt++) = 50; // Requested IP Address
	*(opt++) = 4; // Length
	memcpy(opt, &offered_ip, 4);
	opt += 4;
	*(opt++) = 54;
	*(opt++) = 4; // Length
	memcpy(opt, &dhcp_server_ip, 4);
	opt += 4;
	*(opt++) = 0xFF;
	// checksum

	// === Checksum IP ===
    ip->checksum = 0;
    uint32_t sum = 0;
    uint16_t *ip_data = (uint16_t *)ip;
    for (size_t i = 0; i < sizeof(ip_header_t)/2; ++i)
        sum += ntohs(ip_data[i]);
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

	int dhcp_size = sizeof(dhcp_packet_no_opt_t) + (opt - (uint8_t *)(dhcp + 1));
    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp;
    for (size_t i = 0; i < sizeof(udp_header_t) + dhcp_size; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < sizeof(udp_header_t) + dhcp_size)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    udp->checksum = htons(~sum);

	syscall_eth_send(packet, total_size);
}
