/*****************************************************************************\
|   === vdisk.c : 2026 ===                                                    |
|                                                                             |
|    Kernel virtual disk implementation                            .pi0iq.    |
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
    uint32_t display_id;
} vdisk_t;

vdisk_t *g_vdisks[AFFT_MAX];

// internal functions

static int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
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

static int vdisk_write(uint32_t afft_id, void *data, uint32_t offset, uint32_t size) {
    if (afft_id >= AFFT_MAX || g_vdisks[afft_id] == NULL) {
        sys_error("vdisk_write: invalid afft_id %d", afft_id);
        return -1;
    }

    vdisk_t *vdisk = g_vdisks[afft_id];

    if (offset + size > vdisk->size) {
        if (vdisk_extend(vdisk, offset + size))
            return -1;
    }

    // kprintf("\e[90mvdisk_write: afft_id=%d, offset=%d, size=%d\e[0m\n", afft_id, offset, size);

    mem_copy(vdisk->data + offset, data, size);

    return size;
} // les appels des disque doive verifier la taille ecrite et pas 0 TODO

static int vdisk_read(uint32_t afft_id, void *buffer, uint32_t offset, uint32_t size) {
    if (afft_id >= AFFT_MAX || g_vdisks[afft_id] == NULL) {
        sys_error("vdisk_read: invalid afft_id %d", afft_id);
        return -1;
    }

    vdisk_t *vdisk = g_vdisks[afft_id];

    uint32_t read_size = size;

    if (offset + size > vdisk->size)
        read_size = vdisk->size > offset ? vdisk->size - offset : 0;

    // kprintf_serial("\e[90mvdisk_read: afft_id=%d, offset=%d, size=%d\e[0m\n", afft_id, offset, size);

    if (read_size > 0)
        mem_copy(buffer, vdisk->data + offset, read_size);

    if (read_size < size)
        mem_set(buffer + read_size, 0, size - read_size);

    return size;
}

static void format_name(char name[16], uint32_t id) {
    str_copy(name, "vdisk");
    int2str(id, name + 5);
}

// public functions

int vdisk_create(void *mem, uint32_t size) { // returns a afft id
    static uint32_t vdisk_counter = 0;

    char display_name[16];
    format_name(display_name, vdisk_counter);

    int afft_id = afft_register(
        AFFT_AUTO,
        vdisk_read,
        vdisk_write,
        NULL,
        display_name
    );

    if (afft_id < 0)
        return -1;

    vdisk_t *vdisk = malloc(sizeof(vdisk_t));
    vdisk->display_id = vdisk_counter++;

    if (mem != NULL) {
        vdisk->data = mem;
        vdisk->size = size;
    } else {
        vdisk->data = NULL;
        vdisk->size = 0;
    }

    g_vdisks[afft_id] = vdisk;
    return afft_id;
}

int vdisk_add_to_dev(uint32_t afft_id) {
    char display_name[16];

    if (afft_id >= AFFT_MAX || g_vdisks[afft_id] == NULL)
        return 1;

    format_name(display_name, g_vdisks[afft_id]->display_id);

    return SID_IS_NULL(kfu_afft_create("/dev", display_name, afft_id));
}

void vdisk_destroy(uint32_t afft_id) {
    if (afft_id >= AFFT_MAX || g_vdisks[afft_id] == NULL)
        return;

    vdisk_t *disk = g_vdisks[afft_id];

    // remove the entry from filesystem
    char path[21];
    str_copy(path, "/dev/");
    format_name(path + 5, disk->display_id);

    sid_t sid = kfu_path_to_sid(SID_ROOT, path);
    if (!SID_IS_NULL(sid))
        fs_cnt_delete(sid);

    afft_unregister(afft_id);

    free(disk->data);
    free(disk);
}

int vdisk_init(void) {
    for (int i = 0; i < AFFT_MAX; i++)
        g_vdisks[i] = NULL;
    return 0;
}
