#include <stdint.h>
#include <stdio.h>
#include <profan/syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <profan/net.h>

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
	uint8_t type;
	uint8_t code;
	uint16_t checksum;
} __attribute__((packed)) icmp_header_t;

uint8_t dest_ip[4] = {0};

void send_ping(uint8_t *router_mac, uint8_t *src_mac, uint8_t *src_ip) {
	uint8_t packet[sizeof(ethernet_header_t) + sizeof(ip_header_t) + sizeof(icmp_header_t) + 4];
	ethernet_header_t *eth_hdr = (ethernet_header_t *)packet;
	memcpy(eth_hdr->dest_mac, router_mac, 6);
	memcpy(eth_hdr->src_mac, src_mac, 6);
	eth_hdr->ether_type = htons(0x0800);
	ip_header_t *ip_hdr = (ip_header_t *)(packet + sizeof(ethernet_header_t));
	ip_hdr->version_ihl = 0x45;
	ip_hdr->tos = 0;
	ip_hdr->total_length = htons(sizeof(ip_header_t) + sizeof(icmp_header_t) + 4);
	ip_hdr->id = htons(0x1234);
	ip_hdr->flags_offset = htons(0x4000);
	ip_hdr->ttl = 64;
	ip_hdr->protocol = 1;
	ip_hdr->checksum = 0;
	memcpy(ip_hdr->src_ip, src_ip, 4);
	// to 8.8.8.8
	memcpy(ip_hdr->dest_ip, dest_ip, 4);
	icmp_header_t *icmp_hdr = (icmp_header_t *)(packet + sizeof(ethernet_header_t) + sizeof(ip_header_t));
	icmp_hdr->type = 8;
	icmp_hdr->code = 0;
	icmp_hdr->checksum = 0;
	uint16_t *icmp_data = (uint16_t *)(icmp_hdr + 1);
	icmp_data[0] = htons(0x1234);
	icmp_data[1] = htons(0x5678);
	icmp_hdr->checksum = 0;
	uint16_t icmp_data_buff[sizeof(icmp_header_t) / 2 + 2];
	memcpy(icmp_data_buff, icmp_hdr, sizeof(icmp_header_t) + 4);
	uint32_t sum = 0;
	for (size_t i = 0; i < sizeof(icmp_data_buff) / 2; i++) {
		sum += ntohs(icmp_data_buff[i]);
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	icmp_hdr->checksum = htons((uint16_t)(~sum));

	uint16_t ip_data_buff[sizeof(ip_header_t) / 2];
	memcpy(ip_data_buff, ip_hdr, sizeof(ip_header_t));
	sum = 0;
	for (size_t i = 0; i < sizeof(ip_data_buff) / 2; i++) {
		sum += ntohs(ip_data_buff[i]);
	}
	sum = (sum >> 16) + (sum & 0xFFFF);
	sum += (sum >> 16);
	ip_hdr->checksum = htons((uint16_t)(~sum));

	syscall_eth_send(packet, sizeof(packet));
}

void treat_args(int argc, char **argv);

int g_eth_id = 0;
eth_info_t g_eth_info;

int main(int argc, char **argv) {
	treat_args(argc, argv);
	g_eth_id = syscall_eth_start();
	if (g_eth_id == 0) {
		fprintf(stderr, "Error: could not open connection with profan eth\n");
		return 1;
	}
	syscall_eth_get_info(g_eth_id, &g_eth_info);
	uint8_t mac[6];
	uint8_t router_mac[6];
	memcpy(mac, g_eth_info.mac, 6);
	memcpy(router_mac, g_eth_info.router_mac, 6);
	if (!memcmp(mac, "\0\0\0\0\0\0", 6)) {
		fprintf(stderr, "Error: no ethernet device found\n");
		return 1;
	}
	if (!memcmp(router_mac, "\0\0\0\0\0\0", 6)) {
		fprintf(stderr, "Error: no router mac address found\n");
		return 1;
	}

	for (int i = 0 ; i < 10; i++) {
		send_ping(router_mac, g_eth_info.mac, (uint8_t *)&g_eth_info.ip);
		uint32_t start = syscall_timer_get_ms();
		while (1) {
			uint32_t now = syscall_timer_get_ms();
			if (now - start > 1000) {
				fprintf(stderr, "Ping timeout.\n");
				break;
			}
			int len = syscall_eth_is_ready(g_eth_id);
			if (len < 0)
				continue;
			if (len < (int)sizeof(ethernet_header_t) + (int)sizeof(ip_header_t) + (int)sizeof(icmp_header_t)) {
				continue; // Not enough data for a valid ICMP packet
			}
			uint8_t *buffer = malloc(len);
			if (!buffer) {
				fprintf(stderr, "Memory allocation failed.\n");
				return 1;
			}
			syscall_eth_recv(g_eth_id, buffer);
			ethernet_header_t *eth_hdr = (ethernet_header_t *)buffer;
			if (eth_hdr->ether_type != htons(0x0800)) {
				free(buffer);
				continue; // Not an IPv4 packet
			}
			ip_header_t *ip_hdr = (ip_header_t *)(buffer + sizeof(ethernet_header_t));
			if (ip_hdr->protocol != 1) {
				free(buffer);
				continue; // Not an ICMP packet
			}
			icmp_header_t *icmp_hdr = (icmp_header_t *)(buffer + sizeof(ethernet_header_t) + sizeof(ip_header_t));
			if (icmp_hdr->type != 0 || icmp_hdr->code != 0) {
				free(buffer);
				continue; // Not an ICMP Echo Reply
			}
			uint16_t *icmp_data = (uint16_t *)(icmp_hdr + 1);
			if (ntohs(icmp_data[0]) == 0x1234 && ntohs(icmp_data[1]) == 0x5678) {
				printf("Ping successful: %d bytes from %d.%d.%d.%d in %dms\n",
				       len - sizeof(ethernet_header_t) - sizeof(ip_header_t) - sizeof(icmp_header_t),
				       ip_hdr->src_ip[0], ip_hdr->src_ip[1],
				       ip_hdr->src_ip[2], ip_hdr->src_ip[3], now - start);
				free(buffer);
				break; // Exit the loop on successful ping
			}
			free(buffer);
		}
	}
	return (0);
}

void treat_args(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "Usage: %s <destination_ip>\n", argv[0]);
		exit(1);
	}
	char *addr = argv[1];
	if (addr[0] == '.' || addr[strlen(addr) - 1] == '.') {
		fprintf(stderr, "Invalid IP address format. Use x.x.x.x\n");
		exit(1);
	}
	int dot_count = 0;
	for (int i = 0; addr[i] != '\0'; i++) {
		if (addr[i] == '.')
			dot_count++;
	}
	if (dot_count != 3) {
		fprintf(stderr, "Invalid IP address format. Use x.x.x.x\n");
		exit(1);
	}
	int parts_idx[4] = {0};
	int current_part = 1;
	for (int i = 0; addr[i] != '\0'; i++) {
		if (addr[i] == '.')
			parts_idx[current_part++] = i + 1;
	}
	for (int i = 0; i < 4; i++) {
		int part_len = 0;
		int idx = parts_idx[i];
		while (addr[idx] != '.' && addr[idx] != '\0') {
			part_len++;
			idx++;
		}
		if (part_len == 0 || part_len > 3) {
			fprintf(stderr, "Invalid IP address format. Use x.x.x.x\n");
			exit(1);
		}
		int part_value = 0;
		for (int k = 0; k < part_len; k++) {
			if (addr[parts_idx[i] + k] < '0' || addr[parts_idx[i] + k] > '9') {
				fprintf(stderr, "Invalid IP address format. Use x.x.x.x\n");
				exit(1);
			}
			part_value = part_value * 10 + (addr[parts_idx[i] + k] - '0');
		}
		if (part_value < 0 || part_value > 255) {
			fprintf(stderr, "Invalid IP address format. Use x.x.x.x\n");
			exit(1);
		}
		dest_ip[i] = (uint8_t)part_value;
	}
}
