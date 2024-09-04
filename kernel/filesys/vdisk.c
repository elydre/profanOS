/*****************************************************************************\
|   === vdisk.c : 2024 ===                                                    |
|                                                                             |
|    Kernel virtual disk functions                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

vdisk_t *vdisk_create(uint32_t initsize) {
    vdisk_t *vdisk = malloc(sizeof(vdisk_t));

    vdisk->size = initsize;
    vdisk->free = malloc(sizeof(uint32_t) * initsize);
    vdisk->used = malloc(sizeof(uint8_t) * initsize);
    vdisk->used_count = 0;

    vdisk->sectors = malloc(sizeof(sector_t) * initsize);
    if (vdisk->sectors == NULL) {
        sys_fatal("could not allocate memory for vdisk");
    }

    for (uint32_t i = 0; i < initsize; i++) {
        vdisk->used[i] = 0;
        vdisk->free[i] = i;
    }

    return vdisk;
}

void vdisk_destroy(vdisk_t *vdisk) {
    free(vdisk->sectors);
    free(vdisk->free);
    free(vdisk->used);
    free(vdisk);
}

int vdisk_note_sector_used(vdisk_t *vdisk, uint32_t sid) {
    if (vdisk->used[SID_SECTOR(sid)] == 1) {
        sys_error("Sector already used");
        return 1;
    }

    if (vdisk->free[vdisk->used_count] != SID_SECTOR(sid)) {
        sys_fatal("Sector %d is not the next free sector", SID_SECTOR(sid));
        return 1;
    }

    vdisk->free[vdisk->used_count] = 42;
    vdisk->used[SID_SECTOR(sid)] = 1;
    vdisk->used_count++;
    return 0;
}

int vdisk_note_sector_unused(vdisk_t *vdisk, uint32_t sid) {
    if (vdisk->used[SID_SECTOR(sid)] == 0) {
        sys_error("Sector already unused");
        return 1;
    }

    vdisk->used_count--;

    vdisk->free[vdisk->used_count] = SID_SECTOR(sid);
    vdisk->used[SID_SECTOR(sid)] = 0;
    return 0;
}

int vdisk_is_sector_used(vdisk_t *vdisk, uint32_t sid) {
    return vdisk->used[SID_SECTOR(sid)];
}

int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
    if (newsize <= vdisk->size) {
        return 1;
    }
    vdisk->free = realloc_as_kernel(vdisk->free, sizeof(uint32_t) * newsize);
    vdisk->used = realloc_as_kernel(vdisk->used, sizeof(uint8_t) * newsize);

    vdisk->sectors = realloc_as_kernel(vdisk->sectors, sizeof(sector_t) * newsize);

    if (vdisk->sectors == NULL) {
        sys_fatal("Could not allocate memory for vdisk");
    }

    for (uint32_t i = vdisk->size; i < newsize; i++) {
        vdisk->used[i] = 0;
        vdisk->free[i] = i;
    }

    vdisk->size = newsize;
    return 0;
}

int vdisk_get_unused_sector(vdisk_t *vdisk) {
    if (vdisk->used_count >= vdisk->size) {
        vdisk_extend(vdisk, vdisk->size + 1000);
    }
    return vdisk->free[vdisk->used_count];
}

int vdisk_write_sector(vdisk_t *vdisk, uint32_t sid, uint8_t *data) {
    if (SID_SECTOR(sid) >= vdisk->size) {
        sys_error("[vdisk_write] Sector out of range");
        return 1;
    }
    mem_copy(vdisk->sectors + SID_SECTOR(sid), data, FS_SECTOR_SIZE);
    return 0;
}

int vdisk_read_sector(vdisk_t *vdisk, uint32_t sid, uint8_t *data) {
    if (SID_SECTOR(sid) >= vdisk->size) {
        sys_error("[vdisk_read] Sector out of range");
        return 1;
    }
    mem_copy(data, vdisk->sectors + SID_SECTOR(sid), FS_SECTOR_SIZE);
    return 0;
}

uint8_t *vdisk_load_sector(vdisk_t *vdisk, uint32_t sid) {
    if (SID_SECTOR(sid) >= vdisk->size) {
        sys_error("[vdisk_load] Sector out of range");
        return NULL;
    }
    uint8_t *data = malloc(FS_SECTOR_SIZE);
    mem_copy(data, vdisk->sectors + SID_SECTOR(sid), FS_SECTOR_SIZE);
    return data;
}

int vdisk_unload_sector(vdisk_t *vdisk, uint32_t sid, uint8_t *data, int save) {
    if (save) {
        vdisk_write_sector(vdisk, sid, data);
    }
    free(data);
    return 0;
}
