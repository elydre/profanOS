#include <stdio.h>
#include <stdlib.h>
#include <profan/syscall.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdint.h>

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

void get_router_mac(uint8_t *mac) {
	FILE *f = fopen("/zada/router_mac", "rb");
	if (!f) {
		fprintf(stderr, "Failed to open /zada/router_mac\n");
		exit(1);
	}
	if (fread(mac, 1, 6, f) != 6) {
		fprintf(stderr, "Failed to read MAC address from /zada/router_mac\n");
		fclose(f);
		exit(1);
	}
	fclose(f);
}

void send_udp(uint16_t src_port, uint16_t dest_port, const char *message, size_t message_len, uint8_t *dest_ip) {
	uint8_t *packet = malloc(sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t) + message_len);

	ethernet_header_t *eth_hdr = (ethernet_header_t *)packet;
	ip_header_t *ip_hdr = (ip_header_t *)(packet + sizeof(ethernet_header_t));
	udp_header_t *udp_hdr = (udp_header_t *)(packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	uint8_t *data = packet + sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t);
	uint8_t router_mac[6];
	get_router_mac(router_mac);
	if (router_mac[0] == 0 && router_mac[1] == 0 && router_mac[2] == 0 &&
	    router_mac[3] == 0 && router_mac[4] == 0 && router_mac[5] == 0) {
		fprintf(stderr, "Router MAC address is not set\n");
		free(packet);
		exit(1);
	}
	syscall_eth_get_mac(eth_hdr->src_mac);
	for (int i = 0; i < 6; i++) {
		eth_hdr->dest_mac[i] = router_mac[i];
	}
	eth_hdr->ether_type = htons(0x0800); // IPv4
	syscall_eth_get_ip(ip_hdr->src_ip);
	for (int i = 0; i < 4; i++) {
		ip_hdr->dest_ip[i] = dest_ip[i];
	}
	ip_hdr->version_ihl = 0x45; // IPv4, IHL = 5
	ip_hdr->tos = 0;
	ip_hdr->total_length = htons(sizeof(ip_header_t) + sizeof(udp_header_t) + message_len);
	ip_hdr->id = htons(0);
	ip_hdr->flags_offset = htons(0x4000); // Don't fragment
	ip_hdr->ttl = 64; // Default TTL
	ip_hdr->protocol = 17; // UDP
	ip_hdr->checksum = 0; // Checksum will be calculated later
	udp_hdr->src_port = htons(src_port);
	udp_hdr->dest_port = htons(dest_port);
	udp_hdr->length = htons(sizeof(udp_header_t) + message_len);
	udp_hdr->checksum = 0; // Checksum will be calculated later
	for (size_t i = 0; i < message_len; i++) {
		data[i] = message[i];
	}

	// === Checksum IP ===
    ip_hdr->checksum = 0;
    uint32_t sum = 0;
    // uint16_t *ip_data = (uint16_t *)ip;
    // for (size_t i = 0; i < sizeof(ip_header_t)/2; ++i)
    //     sum += ntohs(ip_data[i]);
    uint16_t ip_data_buff[sizeof(ip_header_t) / 2];
    memcpy(ip_data_buff, ip_hdr, sizeof(ip_header_t));
    for (size_t i = 0; i < sizeof(ip_data_buff) / 2; i++) {
        sum += ntohs(ip_data_buff[i]);
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    ip_hdr->checksum = htons(~sum);

    // === Checksum UDP ===
    sum = 0;
    // Pseudo-header
    sum += (ip_hdr->src_ip[0] << 8) | ip_hdr->src_ip[1];
	sum += (ip_hdr->src_ip[2] << 8) | ip_hdr->src_ip[3];
	sum += (ip_hdr->dest_ip[0] << 8) | ip_hdr->dest_ip[1];
	sum += (ip_hdr->dest_ip[2] << 8) | ip_hdr->dest_ip[3];

    sum += ip_hdr->protocol;
    sum += ntohs(udp_hdr->length);

	size_t udp_len = sizeof(udp_header_t) + message_len;
    // UDP + DHCP
    uint8_t *udp_data = (uint8_t *)udp_hdr;
    for (size_t i = 0; i < udp_len; i += 2) {
        uint16_t word = udp_data[i] << 8;
        if (i + 1 < udp_len)
            word |= udp_data[i + 1];
        sum += word;
    }
    sum = (sum & 0xFFFF) + (sum >> 16);
    sum += (sum >> 16);
    udp_hdr->checksum = htons(~sum);


	// Send the packet
	syscall_eth_send(packet, sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(udp_header_t) + message_len);
	free(packet);
}

int main(int argc, char **argv) {
	// 1 arg
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <message>\n", argv[0]);
		return 1;
	}
	char *message = argv[1];
	if (strlen(message) == 0) {
		fprintf(stderr, "%s: message cannot be empty\n", argv[0]);
		return 1;
	}
	if (strlen(message) > 100) {
		fprintf(stderr, "%s: message is too long (max 100 characters)\n", argv[0]);
		return 1;
	}
	uint8_t mac[6];
	uint8_t ip[4];
	syscall_eth_get_mac(mac);
	if (mac[0] == 0 && mac[1] == 0 && mac[2] == 0 &&
	    mac[3] == 0 && mac[4] == 0 && mac[5] == 0) {
		fprintf(stderr, "%s: no MAC address found\n", argv[0]);
		return 1;
	}
	syscall_eth_get_ip(ip);
	if (ip[0] == 0 && ip[1] == 0 && ip[2] == 0 && ip[3] == 0) {
		fprintf(stderr, "%s: no IP address found\n", argv[0]);
		return 1;
	}
	uint8_t dest_ip[4] = {82, 64, 162, 243};
	send_udp(42420, 42420, message, strlen(message), dest_ip);
	return 0;
}
