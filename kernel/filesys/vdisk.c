/*****************************************************************************\
|   === vdisk.c : 2025 ===                                                    |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

typedef struct {
    uint8_t *data;
    uint32_t size;
} vdisk_t;

vdisk_t *g_vdisk = NULL;

void vdisk_init(void) {
    if (g_vdisk)
        return;

    g_vdisk = malloc(sizeof(vdisk_t));
    g_vdisk->data = NULL;
    g_vdisk->size = 0;
}

void vdisk_destroy(void) {
    if (!g_vdisk)
        return;

    if (g_vdisk->data)
        free(g_vdisk->data);

    free(g_vdisk);
    g_vdisk = NULL;
}

#define DISK_EXTEND_SIZE SECTOR_SIZE * 1024

int vdisk_extend(uint32_t newsize) {
    if (!g_vdisk)
        return -1;

    if (newsize <= g_vdisk->size)
        return 0;

    newsize += DISK_EXTEND_SIZE - (newsize % DISK_EXTEND_SIZE);

    uint8_t *newdata = realloc(g_vdisk->data, newsize);
    if (!newdata)
        return -1;

    mem_set(newdata + g_vdisk->size, 0, newsize - g_vdisk->size);

    g_vdisk->data = newdata;
    g_vdisk->size = newsize;

    return 0;
}

int vdisk_write(void *data, uint32_t size, uint32_t offset) {
    if (!g_vdisk)
        return -1;

    if (offset + size > g_vdisk->size) {
        if (vdisk_extend(offset + size))
            return -1;
    }

    mem_copy(g_vdisk->data + offset, data, size);

    return 0;
}

int vdisk_read(void *buffer, uint32_t size, uint32_t offset) {
    if (!g_vdisk)
        return -1;

    if (offset + size > g_vdisk->size)
        return -1;

    mem_copy(buffer, g_vdisk->data + offset, size);
    return 0;
}
