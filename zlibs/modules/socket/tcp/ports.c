#include "utils.h"
#include <modules/socket.h>
#include "tcp.h"

static uint8_t bitmap[0xFFFF / 8] = {0};

int tcp_is_port_free(uint16_t port) {
    return !((bitmap[port >> 3] >> (port & 7)) & 1);
}

void tcp_lock_port(uint16_t port) {
    bitmap[port >> 3] |= 1 << (port & 7);
}

void tcp_free_port(uint16_t port) {
    bitmap[port >> 3] &= ~(1 << (port & 7));
}

uint16_t tcp_get_free_port() {
    uint16_t res = CLT_PORT_START + (tcp_rand32() & 0x3FF);
    while (res <= CLT_PORT_END) {
        if (!tcp_is_port_free(res))
            res++;
        return htons(res);
    }
    return 0;
}
