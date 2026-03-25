/*****************************************************************************\
|   === eth.h : 2026 ===                                                      |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef ETH_ID
#define ETH_ID 7

#include <stdint.h>

typedef struct eth_info_t {
    uint32_t net_mask;
    uint32_t router_ip;
    uint8_t router_mac[8]; // ignores last 2 bytes

    uint32_t ip;
    uint8_t mac[8]; // ignores last 2 bytes
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

// syscalls

#ifndef _KERNEL_MODULE

uint32_t modeth_start();
void     modeth_end(uint32_t id);
int      modeth_send(void *data, uint16_t len);
int      modeth_is_ready(uint32_t id);
void     modeth_recv(uint32_t id, void *data);
void     modeth_get_info(uint32_t id, struct eth_info_t *info);
void     modeth_set_info(uint32_t id, struct eth_info_t *info);
uint32_t modeth_get_transaction();

extern int profan_syscall(uint32_t id, ...);

#undef  _pscall
#define _pscall(module, id, ...) \
    profan_syscall(((module << 24) | id), __VA_ARGS__)

#define modeth_start()  ((uint32_t) _pscall(ETH_ID, 0, 0))
#define modeth_end(id)  ((void) _pscall(ETH_ID, 1, id))
#define modeth_send(data, len)  ((int) _pscall(ETH_ID, 2, data, len))
#define modeth_is_ready(id)  ((int) _pscall(ETH_ID, 3, id))
#define modeth_recv(id, data)  ((void) _pscall(ETH_ID, 4, id, data))
#define modeth_get_info(id, info)  ((void) _pscall(ETH_ID, 5, id, info))
#define modeth_set_info(id, info)  ((void) _pscall(ETH_ID, 6, id, info))
#define modeth_get_transaction()  ((uint32_t) _pscall(ETH_ID, 7, 0))

#else // _KERNEL_MODULE defined

void     eth_recv_packet(const void *addr, uint16_t p_len);
void     eth_register_nic(int (*on_send)(const void *addr_phys, uint16_t len), const uint8_t *mac);
void     eth_listeners_add_packet(const void *addr, int len);

uint32_t eth_start();
void     eth_end(uint32_t id);
int      eth_send(void *data, uint16_t len);
int      eth_is_ready(uint32_t id);
void     eth_recv(uint32_t id, void *data);
void     eth_get_info(uint32_t id, struct eth_info_t *info);
void     eth_set_info(uint32_t id, struct eth_info_t *info);
uint32_t eth_get_transaction();

#ifndef _ETH_C

#undef get_func_addr
#define get_func_addr ((uint32_t (*)(uint32_t, uint32_t)) *(uint32_t *) 0x1ffffb)

#define eth_start ((uint32_t (*)(void)) get_func_addr(ETH_ID, 0))
#define eth_end ((void (*)(uint32_t)) get_func_addr(ETH_ID, 1))
#define eth_send ((int (*)(void *, uint16_t)) get_func_addr(ETH_ID, 2))
#define eth_is_ready ((int (*)(uint32_t)) get_func_addr(ETH_ID, 3))
#define eth_recv ((void (*)(uint32_t, void *)) get_func_addr(ETH_ID, 4))
#define eth_get_info ((void (*)(uint32_t, struct eth_info_t *)) get_func_addr(ETH_ID, 5))
#define eth_set_info ((void (*)(uint32_t, struct eth_info_t *)) get_func_addr(ETH_ID, 6))
#define eth_get_transaction ((uint32_t (*)(void)) get_func_addr(ETH_ID, 7))

#define eth_recv_packet ((void (*)(const void *, uint16_t)) get_func_addr(ETH_ID, 8))
#define eth_register_nic ((void (*)(int (*)(const void *, uint16_t), const uint8_t *)) get_func_addr(ETH_ID, 9))
#define eth_listeners_add_packet ((void (*)(const void *, int)) get_func_addr(ETH_ID, 10))

#endif // _ETH_C
#endif // _KERNEL_MODULE

#endif
