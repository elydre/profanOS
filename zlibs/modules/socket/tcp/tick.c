#include <minilib.h>

#include "tcp.h"

void tcp_tick(tcp_t *sock) {
    switch (sock->state) {
        case TCP_STATE_SYN_SENT:
            // !TODO implement timeout for SYN_SENT
            break;
        case TCP_STATE_OPEN:
            // !TODO implement timeout for OPEN
            break;
        default:
            break;
    }
}