#include "socket.h"

socket_t *sockets = NULL;
int sockets_len = 0;

int socket_socket(int domain, int type, int protocol) {
	int supported[][3] = {
		{AF_INET, SOCK_STREAM, 0},
		{-1, 0, 0}
	};
	int i = -1;
	int res = -1;
	while (supported[++i][0] != -1) {
		if (supported[0] != domain || supported[1] != type || supported[2] != protocol)
			continue;
		sockets = realloc(sockets, sizeof(socket_t) * (sockets_len + 1));
		sockets[sockets_len].type[0] = domain;
		sockets[sockets_len].type[1] = type;
		sockets[sockets_len].type[2] = protocol;

		sockets_len++;
		break;
	}
	return res;
}
