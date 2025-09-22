// @LINK: libmlw
#include <mlw.h>
#include <stdio.h>
#include <string.h>

int main() {
    mlw_instance_t *inst = mlw_open(mlw_ip_from_str("146.190.62.39"), 80);
    if (!inst) {
        printf("Failed to open MLW instance\n");
        return 1;
    }

    const char *req =  "GET / HTTP/1.0\r\nHost: httpforever.com\r\n\r\n";
    if (mlw_tcp_send(inst, (void *)req, strlen(req))) {
        printf("Failed to send TCP request\n");
        mlw_tcp_close(inst, 1000);
        return 1;
    }

	while (1) {
    	int size = 0;
    	char *packet = mlw_tcp_recv(inst, &size, 500); // 10s timeout
    	if (size <= 0) {
			break;
    	} else {
    	    printf("HTTP Response (%d bytes):\n%.*s\n", size, size, packet);
    	}
	}

    mlw_tcp_close(inst, 1000);
    return 0;
}
