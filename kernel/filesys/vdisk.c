#include <butterfly.h>
#include <minilib.h>


vdisk_t *vdisk_create(uint32_t initsize) {
    vdisk_t *vdisk = malloc(sizeof(vdisk_t));
    vdisk->size = initsize;
    vdisk->sectors = malloc(sizeof(sector_t *) * initsize);
    vdisk->free = malloc(sizeof(uint32_t) * initsize);
    vdisk->used = malloc(sizeof(uint8_t) * initsize);
    vdisk->used_count = 0;
    for (uint32_t i = 0; i < initsize; i++) {
        vdisk->sectors[i] = malloc(sizeof(sector_t));
        if (vdisk->sectors[i] == NULL) {
            printf("error: could not allocate sector %d\n", i);
            exit(1);
        }
        vdisk->used[i] = 0;
        vdisk->free[i] = i;
    }
    return vdisk;
}

void vdisk_destroy(vdisk_t *vdisk) {
    for (uint32_t i = 0; i < vdisk->size; i++) {
        free(vdisk->sectors[i]);
    }
    free(vdisk->sectors);
    free(vdisk->free);
    free(vdisk->used);
    free(vdisk);
}

int vdisk_note_sector_used(vdisk_t *vdisk, sid_t sid) {
    if (vdisk->used[sid.sector] == 1) {
        printf("d%ds%d already used\n", sid.device, sid.sector);
        return 1;
    }

    if (vdisk->free[vdisk->used_count] != sid.sector) {
        printf("cannot use s%d, expected s%d\n",
            sid.sector,
            vdisk->free[vdisk->used_count]
        );
        exit(1);
        return 1;
    }

    vdisk->free[vdisk->used_count] = 42;
    vdisk->used[sid.sector] = 1;
    vdisk->used_count++;
    return 0;
}

int vdisk_note_sector_unused(vdisk_t *vdisk, sid_t sid) {
    if (vdisk->used[sid.sector] == 0) {
        printf("d%ds%d already unused\n", sid.device, sid.sector);
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
        printf("vdisk %p is full\n", vdisk);
        return -1;
    }
    return vdisk->free[vdisk->used_count];
}

int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
    if (newsize <= vdisk->size) {
        printf("vdisk %p already has %d sectors\n", vdisk, vdisk->size);
        return 1;
    }
    vdisk->sectors = realloc(vdisk->sectors, sizeof(sector_t *) * newsize);
    vdisk->free = realloc(vdisk->free, sizeof(uint32_t) * newsize);
    vdisk->used = realloc(vdisk->used, sizeof(uint8_t) * newsize);
    for (uint32_t i = vdisk->size; i < newsize; i++) {
        vdisk->sectors[i] = malloc(sizeof(sector_t));
        vdisk->used[i] = 0;
        vdisk->free[i] = i;
    }
    vdisk->size = newsize;
    return 0;
}

int vdisk_write_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data) {
    if (sid.sector >= vdisk->size) {
        printf("d%ds%d out of range\n", sid.device, sid.sector);
        return 1;
    }
    memcpy(vdisk->sectors[sid.sector], data, sizeof(sector_t));
    return 0;
}

uint8_t *vdisk_load_sector(vdisk_t *vdisk, sid_t sid) {
    if (sid.sector >= vdisk->size) {
        printf("d%ds%d out of range\n", sid.device, sid.sector);
        return NULL;
    }
    return (uint8_t *) vdisk->sectors[sid.sector];
}

int vdisk_unload_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data, int save) {
    if (sid.sector >= vdisk->size) {
        printf("d%ds%d out of range\n", sid.device, sid.sector);
        return 1;
    }
    (void) save;
    (void) data;
    return 0;
}
