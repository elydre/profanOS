#ifndef UTILS_SOCK_H
#define UTILS_SOCK_H

#include <stdint.h>

uint16_t ntohs(uint16_t x);
uint32_t ntohl(uint32_t x);

#define htons ntohs
#define htonl ntohl

#endif
