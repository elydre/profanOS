#include <sys/socket.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <stdio.h>

#define ERR_UNKNOWN_REQUEST 0xFFFF
#define ERR_UPDATING 0xFF00
#define ERR_INV_ID 0xFF01
#define ERR_OUT_RANGE 0xFF03
#define ERR_TOO_LONG 0xFF04

#define GET_ID 0x01
#define GET_ID_RSP 0x02

#define GET_INFO 0x03
#define GET_INFO_RSP 0x04

#define READ_PART 0x07
#define READ_PART_RSP 0x08

#define GET_DEP 0x09
#define GET_DEP_RSP 0x0A

#define MAIN_PORT 42024
#define MAIN_IP   "asqel.ddns.net"
#define STP_PKG_SIZE 1300

int G_FD;

long get_pkg_id(uint64_t xid, const char *name) {
    if (strlen(name) > STP_PKG_SIZE - 8) {
        fprintf(stderr, "[get_pkg_id] EUUU nom d'entre trop long\n");
        return -1;
    }

    uint8_t buf[STP_PKG_SIZE];
    uint64_t r = GET_ID;

    int s = strlen(name);

    memcpy(buf, &xid, 6);
    memcpy(buf + 6, &r, 2);
    memcpy(buf + 8, name, s);

    if (send(G_FD, buf, 8 + s, 0) == -1) {
        fprintf(stderr, "[get_pkg_id] EUUU send marche plus\n");
        return -1;
    }

    int rlen = recv(G_FD, buf, 1300, 0);

    if (rlen != 16) {
        fprintf(stderr, "[get_pkg_id] EUUU longeur invalide\n");
        return -1;
    }

    r = 0;
    memcpy(&r, buf, 6);

    if (r != xid) {
        fprintf(stderr, "[get_pkg_id] EUUU c'est pas le bon xid en faite\n");
        return -1;
    }

    r = 0;
    memcpy(&r, buf + 6, 2);

    if (r != GET_ID_RSP) {
        fprintf(stderr, "[get_pkg_id] EUUU c'est pas le bon type en faite\n");
        return -1;
    }

    memcpy(&r, buf + 8, 8);
    return r;
}


int main(void) {
    struct hostent *info = gethostbyname(MAIN_IP);

    if (!info) {
		fprintf(stderr, MAIN_IP " pas trouve\n");
		return 1;
	}

    printf(MAIN_IP ": %d.%d.%d.%d\n",
		(uint8_t)info->h_addr_list[0][0],
		(uint8_t)info->h_addr_list[0][1],
		(uint8_t)info->h_addr_list[0][2], 
		(uint8_t)info->h_addr_list[0][3]
	);

    int fd = socket(AF_INET, SOCK_DGRAM, 0);
    G_FD = fd;

	if (fd < 0) {
		return 1;
	}

	struct sockaddr_in addr;
	addr.sin_family = AF_INET;
	memcpy(&addr.sin_addr, info->h_addr_list[0], 4);
	addr.sin_port = htons(MAIN_PORT);

	if (connect(fd, (void *)&addr, sizeof(addr))) {
        fprintf(stderr, "erreur de connection ta mere\n");
		close(fd);
		return 1;
	}

    printf("aaaa %ld\n", get_pkg_id(99, "tcc"));

    close(fd);
    return 0;
}
