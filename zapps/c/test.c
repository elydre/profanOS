// @LINK: libmlw

#include <mlw.h>
#include <stdio.h>

int main() {
	mlw_init();
	printf("%d\n", mlw_tcp_send(4242, htonl((45UL << 24) + (79UL << 16) + (112UL << 8) + 203), 4242, 236, 0, 2, NULL, 0));
	return 0;
}
