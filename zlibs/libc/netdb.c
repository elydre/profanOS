/*****************************************************************************\
|   === netdb.c : 2026 ===                                                    |
|                                                                             |
|    Implementation of netdb functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <netdb.h>
#include <stdio.h>
#include <time.h>
#include <poll.h>

int h_errno = 0;

#define DNS_IP_PATH "/sys/dns/ip.conf"
#define DNS_LOCAL_PATH "/sys/dns/local.conf"

static uint32_t get_dns_ip() {
	uint32_t res = htonl(0x08080808);

	FILE *f = fopen(DNS_IP_PATH, "r");
	if (!f)
		return res;

	char line[256] = {0};
	int ret = fread(line, 1, 255, f);
	if (ret < 0) {
		fclose(f);
		return res;
	}

	line[ret] = '\0';

	uint32_t res2 = 0;
	if (inet_pton(AF_INET, line, &res2)) {
		fclose(f);
		return res;
	}

	res = res2;
	fclose(f);
	return res;
}

static uint32_t get_local(char *domain, int *err) {
	*err = 0;
	FILE *f = fopen(DNS_LOCAL_PATH, "r");

	while (1) {
		char *line = NULL;
		size_t len = 0;
		ssize_t ret = getline(&line, &len, f);
		if (ret == -1) {
			free(line);
			break;
		}
		if (line[0] == '/' && line[1] == '/') {
			free(line);
			continue;
		}
		char *sep = strchr(line, ':');
		if (!sep) {
			free(line);
			continue;
		}
		*sep = '\0';
		if (strcmp(domain, line)) {
			free(line);
			continue;
		}
		uint32_t dest_ip = 0;
		if (inet_pton(AF_INET, sep + 1, &dest_ip)) {
			free(line);
			continue;
		}
		fclose(f);
		free(line);
		return dest_ip;
	}
	*err = 1;
	fclose(f);
	return 0;
}


static int get_parts(uint8_t parts[256], char *domain) {
    int i = 0;
    char *token = strtok(domain, ".");

    while (token) {
        if (i >= 255)
            return 1;
        int len = strlen(token);
        if (len > 63)
            return 1;
        if (len + 1 + i >= 255)
            return 1;
        parts[i++] = (uint8_t)len;
        memcpy(&parts[i], token, len);
        i += len;
        token = strtok(NULL, ".");
    }
    parts[i] = 0;
    return 0;
}

static int send_query(int fd, char *domain, uint16_t id) {
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

static int parse_packet(uint8_t *packet, int packet_len, uint16_t id, uint32_t *ip) {
    if (packet_len < 12)
        return 1;
    uint16_t rid = (packet[0] << 8) | packet[1];
    uint16_t flags = (packet[2] << 8) | packet[3];
    uint16_t qdcount = (packet[4] << 8) | packet[5];
    uint16_t ancount = (packet[6] << 8) | packet[7];
    uint16_t nscount = (packet[8] << 8) | packet[9];
    uint16_t arcount = (packet[10] << 8) | packet[11];
    (void)arcount;
    (void)nscount;
    if (id != rid)
        return 1;
    if (qdcount != 1)
        return 1;
    int i = 12;
    while (i < packet_len) {
        uint8_t part_len = packet[i];
        i += part_len + 1;
        if (part_len == 0)
            break;
    }
    i += 4;
    if (flags & 0b1111 || ancount == 0)
        return 1;
    if (i >= packet_len)
        return 1;
    if ((packet[i] & 0xc0) == 0xc0)
        i += 2;
    else {
        while (i < packet_len) {
            uint8_t part_len = packet[i];
            i += part_len + 1;
            if (part_len == 0)
                break;
        }
    }
    if (i + 1 >= packet_len)
        return 1;
    uint16_t rtype = (packet[i] << 8) | packet[i + 1];
    if (rtype != 1)
        return 1;
    i += 2;
    if (i + 1 >= packet_len)
        return 1;
    uint16_t rclass = (packet[i] << 8) | packet[i + 1];
    if (rclass != 1)
        return 1;
    i += 2;
    i += 4; // skip ttl
    if (i + 1 >= packet_len)
        return 1;

    uint16_t rlen = (packet[i] << 8) | packet[i + 1];
    if (rlen != 4)
        return 1;
    i += 2;
    if (i + 3 >= packet_len)
        return 1;
    *ip = packet[i];
    *ip |= packet[i + 1] << 8;
    *ip |= packet[i + 2] << 16;
    *ip |= packet[i + 3] << 24;
    return 0;
}

static int get_response(int fd, uint16_t id, uint32_t *ip) {
    uint8_t buffer[1024];

    struct pollfd fds;
    fds.fd = fd;
    fds.events = POLLIN;
    fds.revents = 0;
    int ret = poll(&fds, 1, 1000 * 5);
    if (ret <= 0)
        return 1;
    int len = recv(fd, buffer, 1024, 0);
    return parse_packet(buffer, len, id, ip);
}

static int get_ip(char *name, uint32_t *ip) {
	int err = 0;
	uint32_t local = get_local(name, &err);
	if (!err)
		return local;
	
    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0) {
        return 1;
    }
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = get_dns_ip();
    addr.sin_port = htons(53);
    if (connect(fd, (void *)&addr, sizeof(addr))) {
        close(fd);
        return 1;
    }
    srand(time(NULL) + (getpid() << 7));
    uint16_t id = rand() % 0xFFFF;
    if (send_query(fd, name, id)) {
        close(fd);
        return 1;
    }
    int ret = get_response(fd, id, ip);
    close(fd);
    return ret;
}

struct hostent *gethostbyname(const char *name) {
    static struct hostent storage;
    static char dup[4096];
    static char iarr[4] = {0};
    static char *arr[2] = {iarr, NULL};

    if (strlen(name) > 4095) {
        h_errno = HOST_NOT_FOUND;
        return NULL;
    }

    strcpy(dup, name);

    struct in_addr addr;
    if (inet_pton(AF_INET, name, &addr) == 1) {
        storage.h_name = dup;
        storage.h_aliases = NULL;
        storage.h_addrtype = AF_INET;
        storage.h_length = 4;
        storage.h_addr_list = arr;
        iarr[0] = addr.s_addr & 0xff;
        iarr[1] = (addr.s_addr >> 8) & 0xff;
        iarr[2] = (addr.s_addr >> 16) & 0xff;
        iarr[3] = (addr.s_addr >> 24) & 0xff;
        return &storage;
    }

    uint32_t ip = 0;
    int ret = get_ip(dup, &ip);
    if (ret) {
        h_errno = HOST_NOT_FOUND;
        return NULL;
    }

    storage.h_name = dup;
    storage.h_aliases = NULL;
    storage.h_addrtype = AF_INET;
    storage.h_length = 4;
    storage.h_addr_list = arr;
    iarr[0] = ip & 0xff;
    iarr[1] = ip >> 8;
    iarr[2] = ip >> 16;
    iarr[3] = ip >> 24;
    return &storage;
}
