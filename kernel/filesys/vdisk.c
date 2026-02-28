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
#include <kernel/afft.h>
#include <minilib.h>
#include <system.h>

typedef struct {
    uint8_t *data;
    uint32_t size;
} vdisk_t;

vdisk_t *g_vdisks[AFFT_MAX];

// internal functions

int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
    if (newsize <= vdisk->size)
        return 0;

    newsize += VDISK_EXTEND_SIZE - (newsize % VDISK_EXTEND_SIZE);

    uint8_t *newdata = realloc(vdisk->data, newsize);

    if (!newdata)
        return -1;

    mem_set(newdata + vdisk->size, 0, newsize - vdisk->size); // TODO maybe not needed ?

    vdisk->data = newdata;
    vdisk->size = newsize;

    return 0;
}

int vdisk_write(uint32_t afft_id, void *data, uint32_t offset, uint32_t size) {
    vdisk_t *vdisk = g_vdisks[afft_id];

    if (vdisk == NULL)
        return -1;

    if (offset + size > vdisk->size) {
        if (vdisk_extend(vdisk, offset + size))
            return -1;
    }

    // kprintf("\e[90mvdisk_write: afft_id=%d, offset=%d, size=%d\e[0m\n", afft_id, offset, size);

    mem_copy(vdisk->data + offset, data, size);

    return 0;
}

int vdisk_read(uint32_t afft_id, void *buffer, uint32_t offset, uint32_t size) {
    vdisk_t *vdisk = g_vdisks[afft_id];

    if (vdisk == NULL)
        return -1;

    uint32_t read_size = size;

    if (offset + size > vdisk->size)
        read_size = vdisk->size > offset ? vdisk->size - offset : 0;

    // kprintf("\e[90mvdisk_read: afft_id=%d, offset=%d, size=%d\e[0m\n", afft_id, offset, size);

    if (read_size > 0)
        mem_copy(buffer, vdisk->data + offset, read_size);

    if (read_size < size)
        mem_set(buffer + read_size, 0, size - read_size);

    return 0;
}

// public functions

int vdisk_create(void) { // returns a afft id
    int afft_id = afft_register(
        AFFT_AUTO,
        vdisk_read,
        vdisk_write,
        NULL
    );

    if (afft_id < 0)
        return -1;

    vdisk_t *vdisk = malloc(sizeof(vdisk_t));

    vdisk->data = NULL;
    vdisk->size = 0;

    g_vdisks[afft_id] = vdisk;

    return afft_id;
}

int vdisk_diskify(void *mem, uint32_t size) { // returns a afft id
    int afft_id = afft_register(
        AFFT_AUTO,
        vdisk_read,
        vdisk_write,
        NULL
    );

    if (afft_id < 0)
        return -1;

    vdisk_t *vdisk = malloc(sizeof(vdisk_t));

    vdisk->data = mem;
    vdisk->size = size;

    g_vdisks[afft_id] = vdisk;

    return afft_id;
}

void vdisk_destroy(int afft_id) {
    vdisk_t *disk = g_vdisks[afft_id];

    free(disk->data);
    free(disk);
}

int vdisk_init(void) {
    for (int i = 0; i < AFFT_MAX; i++)
        g_vdisks[i] = NULL;
    return 0;
}
