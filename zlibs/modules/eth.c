/*****************************************************************************\
|   === eth.c : 2026 ===                                                      |
|                                                                             |
|    Ethernet packet transfer as kernel module                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/process.h>
#include <kernel/scubasuit.h>
#include <minilib.h>
#include <system.h>

#define _ETH_C
#include <modules/eth.h>

/*

struct eth_info_t {
    uint32_t net_mask;
    uint32_t router_ip;
    uint8_t router_mac[6];

    uint32_t ip;
    uint32_t mac;
};

syscalls:
    int eth_start() -> returns a id of the listner (can have multiple by process) (0 == error)
    void eth_end(int id)
    int eth_send(void *data, uint16_t len)
    int eth_is_ready(int id) -> -1 if not ready to recv, else returns the size of the next packet
    void eth_recv(int id, void *data)
    void eth_get_info(int id, struct eth_info_t *info)
    void eth_set_info(int id, struct eth_info_t *info)

*/

eth_info_t eth_info = (eth_info_t){0};

// functions used by cards drivers

static int (*g_on_send)(const void *addr_phys, uint16_t len);

void eth_register_nic(int (*on_send)(const void *addr_phys, uint16_t len), const uint8_t *mac) {
    if (!on_send || !mac) {
        sys_warning("eth_register_nic invalid parameters");
        return ;
    }
    g_on_send = on_send;
    mem_copy(eth_info.mac, mac, 6);
    return ;
}

// functions used as syscalls

static eth_listener_t *listeners = NULL;
static int listeners_len = 0;
static uint32_t last_id = 0;

static void remove_listener_at(int idx) {
    for (int i = 0; i < listeners[idx].len; i++)
        free(listeners[idx].packets[i].data);
    free(listeners[idx].packets);
    for (int i = idx; i < listeners_len - 1; i++)
        listeners[i] = listeners[i + 1];
    listeners_len--;
    listeners = realloc(listeners, sizeof(eth_listener_t) * listeners_len);
}

static eth_listener_t *get_listener(uint32_t id, int pid) {
    if (id == 0)
        return NULL;
    for (int i = 0; i < listeners_len; i++) {
        if (listeners[i].id == id) {
            if (pid < 0 || pid == listeners[i].pid)
                return &listeners[i];
        }
    }
    return NULL;
}

uint32_t eth_start() {
    eth_listener_t *tmp = realloc(listeners, sizeof(eth_listener_t) * (listeners_len + 1));
    if (tmp == NULL)
        return 0;
    listeners = tmp;

    eth_listener_t *lis = &listeners[listeners_len];
    listeners_len++;

    lis->len = 0;
    lis->packets = NULL;
    lis->pid = process_get_pid();
    lis->id = ++last_id;
    if (lis->id == 0)
        lis->id = ++last_id;
    return last_id;
}

void eth_end(uint32_t id) {
    if (id == 0)
        return ;
    for (int i = 0; i < listeners_len; i++) {
        if (listeners[i].id == id && listeners[i].pid == (int)process_get_pid()) {
            remove_listener_at(i);
            break;
        }
    }
}

int eth_send(void *data, uint16_t len) {
    if (len < 6)
        return 1;
    const void *addr_phys = scuba_call_phys((void *) data);

    int dest_type = 0;
    if (!mem_cmp(data, (uint8_t *)&eth_info.mac, 6))
        dest_type = 1;
    if (!mem_cmp(data, "\xFF\xFF\xFF\xFF\xFF\xFF", 6))
        dest_type = 2;

    if (dest_type)
        eth_recv_packet(data, len);

    if (dest_type == 1)
        return 0;

    if (g_on_send)
        return g_on_send((const void *) addr_phys, len);
    else
        sys_warning("eth_send no device found inited");
    return 1;
}

int eth_is_ready(uint32_t id) {
    eth_listener_t *lis = get_listener(id, process_get_pid());
    if (lis == NULL)
        return -1;
    if (lis->len == 0)
        return -1;
    return lis->packets[0].len;
}

void eth_recv(uint32_t id, void *data) {
    eth_listener_t *lis = get_listener(id, process_get_pid());
    if (lis == NULL)
        return ;
    if (lis->len == 0)
        return ;
    mem_copy(data, lis->packets[0].data, lis->packets[0].len);

    free(lis->packets[0].data);
    for (int i = 0; i < lis->len - 1; i++)
        lis->packets[i] = lis->packets[i + 1];
    lis->len--;
    lis->packets = realloc(lis->packets, sizeof(eth_packet_t) * lis->len);
}

void eth_get_info(uint32_t id, struct eth_info_t *info) {
    (void)id;
    *info = eth_info;
}

void eth_set_info(uint32_t id, struct eth_info_t *info) {
    (void)id;
    eth_info = *info;
}

void eth_recv_packet(const void *addr, uint16_t len) {
    for (int i = 0; i < listeners_len; i++) {
        eth_listener_t *lis = &listeners[i];

        lis->len++;
        lis->packets = realloc(lis->packets, sizeof(eth_packet_t) * (lis->len));
        lis->packets[lis->len - 1].data = malloc(sizeof(char) * len);
        mem_copy(lis->packets[lis->len - 1].data, addr, len);
        lis->packets[lis->len - 1].len = len;
    }
}

uint32_t eth_get_transaction() {
    static uint32_t _id = 1;

    return _id++;
}

void __atdeath(int pid) {
    int i = 0;
    while (i < listeners_len) {
        if (listeners[i].pid == pid) {
            remove_listener_at(i);
        }
        else
            i++;
    }
}

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4,   // magic

    // syscalls
    eth_start,
    eth_end,
    eth_send,
    eth_is_ready,
    eth_recv,
    eth_get_info,
    eth_set_info,
    eth_get_transaction,

    // module interface
    eth_recv_packet,
    eth_register_nic,
};
