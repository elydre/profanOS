#ifndef SOCKET_MOD_H
#define SOCKET_MOD_H

#include <sys/socket.h>
#include <system.h>
#include <modules/filesys.h>
#include <net.h>
#include <kernel/process.h>
#include <minilib.h>

typedef struct {
	uint32_t type; // domain | type << 8 | protcol << 16
	void *data;
	int parent_pid;
	int ref_count;
	int fd;
} socket_t;

#define CLT_PORT_START 32768
#define CLT_PORT_END 60999

extern socket_t *sockets;
extern int sockets_len;

int socket_socket(int domain, int type, int protocol);
int socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int socket_listen(int sockfd, int backlog);
int socket_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);

void socket_tick(int len, uint8_t *packet);

// TODO fix this with a include fmopen (pf4 cant make complete headers)
enum {
    TYPE_FREE = 0,
    TYPE_FILE,
    TYPE_AFFT,
    TYPE_DIR,
    TYPE_PPRD, // read pipe
    TYPE_PPWR, // write pipe
    TYPE_SOCK
};

typedef struct {
    uint8_t type;
    uint32_t sid;
    int flags;

    uint32_t offset;  // file and afft

    union {
        int          afft_id; // afft
        struct pipe_data_t *pipe;    // pipe
        char        *path;    // dir (for fchdir)
        int          sock_id; // socket
    };
} fd_data_t;

#endif
