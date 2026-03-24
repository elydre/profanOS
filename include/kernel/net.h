/*****************************************************************************\
|   === net.h : 2026 ===                                                      |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef NET_H
#define NET_H

#include <minilib.h>

typedef struct eth_info_t {
    uint32_t net_mask;
    uint32_t router_ip;
    uint8_t router_mac[8]; //ignores last 2 bytes

    uint32_t ip;
    uint8_t mac[8]; //ignores last 2 bytes
} eth_info_t;

typedef struct {
    int len;
    uint8_t *data;
} eth_packet_t;

typedef struct eth_listener_t {
    int pid;
    int len;
    eth_packet_t *packets;
    uint32_t id;
} eth_listener_t;

extern eth_info_t eth_info;

void eth_recv_packet(const void *addr, uint16_t p_len);
int  eth_send_packet(const void *addr, uint16_t p_len);
void eth_register_nic(int (*on_send)(const void *addr_phys, uint16_t len), const uint8_t *mac);
void eth_listeners_add_packet(const void *addr, int len);
void I_listener_remove(int pid);

// syscalls
uint32_t eth_start();
void eth_end(uint32_t id);
int eth_send(void *data, uint16_t len);
int eth_is_ready(uint32_t id);
void eth_recv(uint32_t id, void *data);
void eth_get_info(uint32_t id, struct eth_info_t *info);
void eth_set_info(uint32_t id, struct eth_info_t *info);
uint32_t eth_get_transaction();

#endif
