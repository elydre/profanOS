#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>
#include <type.h>


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

int vdisk_note_sector_used(vdisk_t *vdisk, sid_t sid) {
    if (vdisk->used[sid.sector] == 1) {
        kprintf("d%ds%d already used\n", sid.device, sid.sector);
        return 1;
    }

    if (vdisk->free[vdisk->used_count] != sid.sector) {
        kprintf("cannot use s%d, expected s%d\n",
            sid.sector,
            vdisk->free[vdisk->used_count]
        );
        sys_fatal("vdisk error");
        return 1;
    }

    vdisk->free[vdisk->used_count] = 42;
    vdisk->used[sid.sector] = 1;
    vdisk->used_count++;
    return 0;
}

int vdisk_note_sector_unused(vdisk_t *vdisk, sid_t sid) {
    if (vdisk->used[sid.sector] == 0) {
        kprintf("d%ds%d already unused\n", sid.device, sid.sector);
        return 1;
    }

    vdisk->free[vdisk->used_count] = sid.sector;
    vdisk->used[sid.sector] = 0;
    vdisk->used_count--;
    return 0;
}

int vdisk_is_sector_used(vdisk_t *vdisk, sid_t sid) {
    return vdisk->used[sid.sector];
}

int vdisk_get_unused_sector(vdisk_t *vdisk) {
    if (vdisk->used_count >= vdisk->size) {
        kprintf("vdisk %p is full\n", vdisk);
        return -1;
    }
    return vdisk->free[vdisk->used_count];
}

int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
    if (newsize <= vdisk->size) {
        kprintf("vdisk %p already has %d sectors\n", vdisk, vdisk->size);
        return 1;
    }
    vdisk->free = realloc(vdisk->free, sizeof(uint32_t) * newsize);
    vdisk->used = realloc(vdisk->used, sizeof(uint8_t) * newsize);

    vdisk->sectors = realloc(vdisk->sectors, sizeof(sector_t) * newsize);

    if (vdisk->sectors == NULL) {
        sys_fatal("could not allocate memory for vdisk");
    }

    for (uint32_t i = vdisk->size; i < newsize; i++) {
        vdisk->used[i] = 0;
        vdisk->free[i] = i;
    }

    vdisk->size = newsize;
    return 0;
}

int vdisk_write_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data) {
    if (sid.sector >= vdisk->size) {
        kprintf("d%ds%d out of range\n", sid.device, sid.sector);
        return 1;
    }
    mem_copy(vdisk->sectors + sid.sector * FS_SECTOR_SIZE, data, FS_SECTOR_SIZE);
    return 0;
}

uint8_t *vdisk_load_sector(vdisk_t *vdisk, sid_t sid) {
    if (sid.sector >= vdisk->size) {
        kprintf("d%ds%d out of range\n", sid.device, sid.sector);
        return NULL;
    }
    return (uint8_t *) (vdisk->sectors + (sid.sector * FS_SECTOR_SIZE));
}

int vdisk_unload_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data, int save) {
    if (sid.sector >= vdisk->size) {
        kprintf("d%ds%d out of range\n", sid.device, sid.sector);
        return 1;
    }
    (void) save;
    (void) data;
    return 0;
}
