#include <profan/syscall.h>
#include "pni_private.h"
#include <stdio.h>

static void build_eth(uint8_t *buffer) {
	ethernet_header_t *eth = (ethernet_header_t *)buffer;

	memcpy(eth->dest_mac, router_mac, 6);
	memcpy(eth->src_mac, self_mac, 6);
	eth->ether_type = htons(0x0800);
}

static void build_ip(uint8_t *buffer, uint8_t *dest_ip, size_t data_len) {
	ip_header_t *ip = (ip_header_t *)(buffer + sizeof(ethernet_header_t));
	*ip = (ip_header_t){0};
	memcpy(ip->dest_ip, dest_ip, 4);
	memcpy(ip->src_ip, self_ip, 4);
	printf("lip %d.%d.%d.%d ata %d\n", self_ip[0], self_ip[1], self_ip[2], self_ip[3], -(buffer - ip->src_ip));
	ip->version_ihl = 0x45;
	ip->tos = 0;
	ip->total_length = htons(sizeof(ip_header_t) + sizeof(udp_header_t) + data_len);
	ip->id = 0;
	ip->flags_offset = htons(0x4000);
	ip->ttl = 64;
	ip->protocol = 17;
}

static void build_udp(uint8_t *buffer, uint8_t *data, size_t data_len, uint16_t src_p, uint16_t dest_p) {
	udp_header_t *udp = (udp_header_t *)(buffer + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	uint8_t *data_ptr = buffer + sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t);

	udp->dest_port = htons(dest_p);
	udp->src_port = htons(src_p);
	udp->checksum = 0;
	udp->length = htons(sizeof(udp_header_t) + data_len);
	memcpy(data_ptr, data, data_len);
}

static void compute_checksum(uint8_t *buffer, size_t data_len) {
	ip_header_t *ip_hdr = (ip_header_t *)(buffer + sizeof(ethernet_header_t));
	udp_header_t *udp_hdr = (udp_header_t *)(buffer + sizeof(ethernet_header_t) + sizeof(udp_header_t));

	uint16_t ip_hdr_data[sizeof(ip_header_t) / 2] ={0};
	uint16_t ip_sum = 0;
	memcpy(ip_hdr_data, buffer + sizeof(ethernet_header_t), sizeof(ip_header_t));
	for (size_t i = 0; i < sizeof(ip_header_t) / 2; i++)
		ip_sum += ntohs(ip_hdr_data[i]);
    ip_sum = (ip_sum & 0xFFFF) + (ip_sum >> 16);
    ip_sum += (ip_sum >> 16);
	ip_hdr->checksum = htons(~ip_sum);


	uint16_t udp_sum = 0;
	udp_sum += (ip_hdr->src_ip[0] << 8) | ip_hdr->src_ip[1];
	udp_sum += (ip_hdr->src_ip[2] << 8) | ip_hdr->src_ip[3];
	udp_sum += (ip_hdr->dest_ip[0] << 8) | ip_hdr->dest_ip[1];
	udp_sum += (ip_hdr->dest_ip[2] << 8) | ip_hdr->dest_ip[3];

	udp_sum += ip_hdr->protocol;
    udp_sum += ntohs(udp_hdr->length);

	size_t udp_len = sizeof(udp_header_t) + data_len;
    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp_hdr;
    for (size_t i = 0; i < udp_len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < udp_len)
            word |= udp_data[i + 1];
        udp_sum += word;
    }
    udp_sum = (udp_sum & 0xFFFF) + (udp_sum >> 16);
    udp_sum += (udp_sum >> 16);
    udp_hdr->checksum = htons(~udp_sum);
}

int pni_send(uint16_t src_p, uint16_t dest_p, uint8_t *data, size_t data_len, uint8_t *dest_ip) {
	if (pni_inited == 0)
		return PNI_ERR_NO_INIT;
	if (data_len > PNI_MAX_PACKET)
		return PNI_ERR_PACKET_SIZE;

	uint8_t *buffer = malloc(sizeof(ethernet_header_t) + sizeof(ip_header_t) * sizeof(udp_header_t) + data_len);
	printf("data len %u\n", data_len);

	build_eth(buffer);
	build_ip(buffer, dest_ip, data_len);
	build_udp(buffer, data, data_len, src_p, dest_p);
	compute_checksum(buffer, data_len);

	syscall_eth_send(buffer, sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t) + data_len);
	free(buffer);
	return 0;
}