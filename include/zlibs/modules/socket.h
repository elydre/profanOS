#ifndef SOCKET_MOD_H
#define SOCKET_MOD_H 8

#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <modules/filesys.h>
#ifdef _KERNEL_MODULE
	#include <system.h>
	#include <net.h>
	#include <kernel/process.h>
	#include <minilib.h>
#endif

typedef struct {
	uint32_t type; // domain | type << 8 | protcol << 16
	void *data;
	int parent_pid;
	int ref_count;
	int fd;
	int do_remove;
} socket_t;

#define CLT_PORT_START 32768
#define CLT_PORT_END 60999

extern socket_t *sockets;
extern int sockets_len;

int socket_socket(int domain, int type, int protocol);
int socket_bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int socket_connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen);
int socket_listen(int sockfd, int backlog);
int socket_accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen);
int socket_close(socket_t *sock);
int socket_close_fd(int fd);

typedef struct {
	int sockfd;
	const void *buf;
	size_t len;
	int flags;
	const struct sockaddr *dest_addr;
	socklen_t addrlen;
} sendto_arg_t;
ssize_t socket_sendto(sendto_arg_t *args);

typedef struct {
	int sockfd;
	void *buf;
	size_t len;
	int flags;
	struct sockaddr *src_addr;
	socklen_t *addrlen;
} recvfrom_arg_t;
ssize_t socket_recvfrom(recvfrom_arg_t *args);

void socket_tick(int len, uint8_t *packet);
socket_t *socket_find_fd(int fd);

// TODO fix this with a include fmopen (pf4 cant make complete headers)


#ifndef _KERNEL_MODULE

extern int profan_syscall(uint32_t id, ...);

#undef _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define socket_socket(a, b, c) ((int) _pscall(SOCKET_MOD_H, 0, a, b, c))
#define socket_bind(a, b, c) ((int) _pscall(SOCKET_MOD_H, 1, a, b, c))
#define socket_connect(a, b, c) ((int) _pscall(SOCKET_MOD_H, 2, a, b, c))
#define socket_sendto(a) ((int) _pscall(SOCKET_MOD_H, 3, a))
#define socket_recvfrom(a) ((int) _pscall(SOCKET_MOD_H, 4, a))

#else 

#define socket_sendto_call ((int (*)(sendto_arg_t *)) get_func_addr(SOCKET_MOD_H, 3))
#define socket_recvfrom_call ((int (*)(recvfrom_arg_t *)) get_func_addr(SOCKET_MOD_H, 4))
#define socket_close_fd_call ((int (*)(int)) get_func_addr(SOCKET_MOD_H, 5))

#endif

#endif
