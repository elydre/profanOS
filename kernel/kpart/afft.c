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
    int (*read) (void *buffer, uint32_t offset, uint32_t size);
    int (*write)(void *buffer, uint32_t offset, uint32_t size);
    int (*cmd)  (uint32_t cmd, void *arg);
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
        int (*read)  (void *buffer, uint32_t offset, uint32_t size),
        int (*write) (void *buffer, uint32_t offset, uint32_t size),
        int (*cmd)   (uint32_t cmd, void *arg)
) {
    if (wanted_id != AFFT_AUTO) {
        if (wanted_id >= AFFT_MAX || g_afft_funcs[wanted_id].busy)
            return -1;
        g_afft_funcs[wanted_id].busy = 1;
        g_afft_funcs[wanted_id].read = read;
        g_afft_funcs[wanted_id].write = write;
        g_afft_funcs[wanted_id].cmd = cmd;
        return wanted_id;
    }

    for (int i = AFFT_RESERVED; i < AFFT_MAX; i++) {
        if (g_afft_funcs[i].busy)
            continue;
        g_afft_funcs[i].busy = 1;
        g_afft_funcs[i].read = read;
        g_afft_funcs[i].write = write;
        g_afft_funcs[i].cmd = cmd;
        return i;
    }

    return -1;
}

int afft_read(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    if (id < AFFT_MAX && g_afft_funcs[id].busy && g_afft_funcs[id].read)
        return g_afft_funcs[id].read(buffer, offset, size);
    return -1;
}

int afft_write(uint32_t id, void *buffer, uint32_t offset, uint32_t size) {
    if (id < AFFT_MAX && g_afft_funcs[id].busy && g_afft_funcs[id].write)
        return g_afft_funcs[id].write(buffer, offset, size);
    return -1;
}

int afft_cmd(uint32_t id, uint32_t cmd, void *arg) {
    if (id < AFFT_MAX && g_afft_funcs[id].busy && g_afft_funcs[id].cmd)
        return g_afft_funcs[id].cmd(cmd, arg);
    return -1;
}
