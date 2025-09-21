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
        return 1;
    }

	while (1) {
    	char resp[4096] = {0};
    	int n = mlw_tcp_recv(inst, resp, sizeof(resp), 10000); // 10s timeout
    	if (n <= 0) {
    	    printf("No response received\n");
			break;
    	} else {
    	    printf("HTTP Response (%d bytes):\n%s\n", n, resp);
    	}
	}

    mlw_tcp_close(inst, 100);
    return 0;
}
