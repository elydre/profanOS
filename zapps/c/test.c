#include <sys/socket.h>
#include <arpa/inet.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int get_parts(uint8_t parts[256], char *domain) {
	int i = 0;
	char *token = strtok(domain, ".");

	while (token) {
		if (i >= 255) {
			fprintf(stderr, "dommain name too long\n");
			return 1;
		}
		int len = strlen(token);
		if (len > 63) {
			fprintf(stderr, "dommain name part too long\n");
			return 1;
		}
		if (len + 1 + i >= 255) {
			fprintf(stderr, "dommain name too long\n");
			return 1;
		}
		parts[i++] = (uint8_t)len;
		memcpy(&parts[i], token, len);
		i += len;
		token = strtok(NULL, ".");
	}
	parts[i] = 0;
	return 0;
}	

int send_query(int fd, char *domain, uint16_t id) {
	uint8_t parts[256];
	if (get_parts(parts, domain))
		return 1;
	
	uint8_t packet[12 + 255 + 1 + 5];
	packet[0] = id >> 8;
	packet[1] = id & 0xff;

	// flags
	packet[2] = 1;
	packet[3] = 0;

	// Q count ?
	packet[4] = 0;
	packet[5] = 1;

	// A count ?
	packet[6] = 0;
	packet[7] = 0;

	// idk
	packet[8] = 0;
	packet[9] = 0;
	packet[10] = 0;
	packet[11] = 0;

	int parts_len = 0;
	while (parts[parts_len])
		parts_len++;
	memcpy(&packet[12], parts, parts_len + 1);
	packet[12 + parts_len + 1 + 0] = 0;
	packet[12 + parts_len + 1 + 1] = 1;
	packet[12 + parts_len + 1 + 2] = 0;
	packet[12 + parts_len + 1 + 3] = 1;
	send(fd, packet, 12 + parts_len + 1 + 5, 0);
	return 0;
}

int parse_packet(uint8_t *packet, int packet_len, uint16_t id, uint32_t *ip) {
	if (packet_len < 12) {
		fprintf(stderr, "malformed response\n");
		return 1;
	}
	uint16_t rid = (packet[0] << 8) | packet[1];
	uint16_t flags = (packet[2] << 8) | packet[3];
	uint16_t qdcount = (packet[4] << 8) | packet[5];
	uint16_t ancount = (packet[6] << 8) | packet[7];
	uint16_t nscount = (packet[8] << 8) | packet[9];
	uint16_t arcount = (packet[10] << 8) | packet[11];
	if (id != rid) {
		fprintf(stderr, "invalid response id\n");
		return 1;
	}
	if (qdcount != 1) {
		fprintf(stderr, "invalid qdcount (%d) expected 1\n", qdcount);
		return 1;
	}
	int i = 12;
	while (i < packet_len) {
		uint8_t part_len = packet[i];
		i += part_len + 1;
		if (part_len == 0)
			break;
	}
	if (flags & 0b1111 || ancount == 0) {
		fprintf(stderr, "domain name not found\n");
		return 1;
	}
	if (i >= packet_len) {
		fprintf(stderr, "malformed packet response\n");
		return 1;
	}
	if ((packet[i]) & 0xc0 == 0xc0)
		i += 2;
	else {
		while (i < packet_len) {
			uint8_t part_len = packet[i];
			i += part_len + 1;
			if (part_len == 0)
				break;
		}
	}
	if (i + 1 >= packet_len) {
		fprintf(stderr, "malformed packet response\n");
		return 1;
	}
	uint16_t rtype = (packet[i] << 8) | packet[i + 1];
	if (rtype != 1) {
		fprintf(stderr, "invalude type in response\n");
		return 1;
	}
	i += 2;
	if (i + 1 >= packet_len) {
		fprintf(stderr, "malformed packet response\n");
		return 1;
	}
	uint16_t rclass = (packet[i] << 8) | packet[i + 1];
	if (rclass != 1) {
		fprintf(stderr, "invalude class in response\n");
		return 1;
	}
	i += 2;
	i += 4; // skip ttl
	if (i + 1 >= packet_len) {
		fprintf(stderr, "malformed packet response\n");
		return 1;
	}

	uint16_t rlen = (packet[i] << 8) | packet[i + 1];
	if (rlen != 4) {
		fprintf(stderr, "invalude len in response\n");
		return 1;
	}
	i += 2;
	if (i + 3 >= packet_len) {
		fprintf(stderr, "malformed packet response\n");
		return 1;
	}
	*ip = packet[i];
	*ip |= packet[i + 1] << 8;
	*ip |= packet[i + 2] << 16;
	*ip |= packet[i + 3] << 24;
	return 0;
}

int print_response(int fd, uint16_t id) {
	uint8_t buffer[1024];

	struct pollfd fds;
	fds.fd = fd;
	fds.events = POLLIN;
	fds.revents = 0;
	int ret = poll(&fds, 1, 1000 * 5);
	if (ret <= 0) {
		fprintf(stderr, "Failed to retrieved domain name\n");
		return 1;
	}
	int len = recv(fd, buffer, 1024, 0);
	uint32_t ip = 0;
	ret = parse_packet(buffer, len, id, &ip);
	if (ret)
		return 1;
	printf("%d.%d.%d.%d\n", ip & 0xff, (ip >> 8) & 0xff, (ip >> 16) & 0xff, ip >> 24);
	return 0;
}

int main(int argc, char **argv) {
	if (argc != 2) {
		fprintf(stderr, "usage: %s <domain name>\n", argv[0]);
		return 1;
	}
	int fd = socket(AF_INET, SOCK_DGRAM, 0);
	if (fd < 0) {
		perror("socket");
		return 1;
	}
	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(0x08080808);
	addr.sin_port = htons(53);
	if (connect(fd, (void *)&addr, sizeof(addr))) {
		perror("connect");
		close(fd);
		return 1;
	}
	srand(time(NULL) + getpid() << 7);
	uint16_t id = rand() % 0xFFFF;
	if (send_query(fd, argv[1], id)) {
		close(fd);
		return 1;
	}
	int ret = print_response(fd, id);
	close(fd);
	return ret;
}
