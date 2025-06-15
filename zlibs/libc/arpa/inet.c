#include <arpa/inet.h>
#include <inttypes.h>

uint32_t htonl(uint32_t hostlong) {
	return ((hostlong & 0xFF000000) >> 24) | ((hostlong & 0x00FF0000) >> 8) |
		   ((hostlong & 0x0000FF00) << 8) | ((hostlong & 0x000000FF) << 24);
}

uint16_t htons(uint16_t hostshort) {
	return (hostshort << 8) | (hostshort >> 8);
}

uint32_t ntohl(uint32_t netlong) {
	return htonl(netlong);
}

uint16_t ntohs(uint16_t netshort) {
	return htons(netshort);
}
