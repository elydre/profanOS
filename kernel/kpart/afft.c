/*****************************************************************************\
|   === afft.c : 2025 ===                                                     |
|                                                                             |
|    Kernel advanced file system functions                         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/afft.h>
#include <minilib.h>

typedef struct {
    int busy;
    int (*read) (uint32_t id, void *buffer, uint32_t offset, uint32_t size);
    int (*write)(uint32_t id, void *buffer, uint32_t offset, uint32_t size);
    int (*cmd)  (uint32_t id, uint32_t cmd, void *arg);
    char name[AFFT_NAME_MAX];
} afft_map_t;

afft_map_t *g_afft_funcs;

int afft_init(void) {
    g_afft_funcs = malloc(sizeof(afft_map_t) * AFFT_MAX);

    for (int i = 0; i < AFFT_MAX; i++)
        g_afft_funcs[i].busy = 0;

    return 0;
}

int afft_register(
        uint32_t wanted_id,
        int (*read)  (uint32_t id, void *buffer, uint32_t offset, uint32_t size),
        int (*write) (uint32_t id, void *buffer, uint32_t offset, uint32_t size),
        int (*cmd)   (uint32_t id, uint32_t cmd, void *arg),
        const char *name
) {
    if (wanted_id == AFFT_AUTO) {
        for (int i = AFFT_RESERVED; i < AFFT_MAX; i++) {
            if (g_afft_funcs[i].busy)
                continue;
            wanted_id = i;
            break;
        }
        if (wanted_id == AFFT_AUTO)
            return -1;
    } else if (wanted_id >= AFFT_MAX || g_afft_funcs[wanted_id].busy)
        return -1;

    g_afft_funcs[wanted_id].busy = 1;
    g_afft_funcs[wanted_id].read = read;
    g_afft_funcs[wanted_id].write = write;
    g_afft_funcs[wanted_id].cmd = cmd;

    if (name)
        str_ncopy(g_afft_funcs[wanted_id].name, name, AFFT_NAME_MAX - 1);

    return wanted_id;
}

int afft_unregister(uint32_t id) {
    if (id >= AFFT_MAX || !g_afft_funcs[id].busy)
        return -1;

    g_afft_funcs[id].busy = 0;
    return 0;
}

int afft_read(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    uint32_t rawid = id & 0x0000FFFF;

    if (rawid < AFFT_MAX && g_afft_funcs[rawid].busy && g_afft_funcs[rawid].read)
        return g_afft_funcs[rawid].read(id, buffer, offset, size);
    return -1;
}

int afft_write(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    uint32_t rawid = id & 0x0000FFFF;

    if (rawid < AFFT_MAX && g_afft_funcs[rawid].busy && g_afft_funcs[rawid].write)
        return g_afft_funcs[rawid].write(id, buffer, offset, size);
    return -1;
}

int afft_cmd(uint32_t id, uint32_t cmd, void *arg) {
    uint32_t rawid = id & 0x0000FFFF;

    if (!(rawid < AFFT_MAX && g_afft_funcs[rawid].busy))
        return (cmd == AFFTC_EXISTS) ? 0 : -1;

    switch (cmd) {
        case AFFTC_EXISTS:
            return 1;
        case AFFTC_GETNAME:
            str_ncopy(arg, g_afft_funcs[rawid].name ? g_afft_funcs[rawid].name :
                        "anonymous", AFFT_NAME_MAX - 1);
            return 0;
    }

    if (!g_afft_funcs[rawid].cmd)
        return -1;

    return g_afft_funcs[rawid].cmd(id, cmd, arg);
}
