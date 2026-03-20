#include "utils.h"

uint16_t ntohs(uint16_t x) {
    return (x >> 8) | (x << 8);
}

uint32_t ntohl(uint32_t x) {
    uint32_t res = x >> 24;

    res |= ((x >> 16) & 0xff) << 8;
    res |= ((x >> 8) & 0xff) << 16;
    res |= (x & 0xff) << 24;
    return res;
}
