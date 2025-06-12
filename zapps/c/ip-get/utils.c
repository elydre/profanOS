#include "ip-get.h"
uint16_t htons(uint16_t n) {
	return (n << 8) | (n >> 8);
}

uint32_t htonl(uint32_t n) {
	return ((n & 0xFF000000) >> 24) | ((n & 0x00FF0000) >> 8) |
	       ((n & 0x0000FF00) << 8) | ((n & 0x000000FF) << 24);
}

uint16_t ntohs(uint16_t n) {
	return htons(n);
}