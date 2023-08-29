#include <kernel/butterfly.h>
#include <drivers/serial.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>


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
        sys_error("sector already used");
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
        sys_error("sector already unused");
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

int vdisk_extend(vdisk_t *vdisk, uint32_t newsize) {
    if (newsize <= vdisk->size) {
        return 1;
    }
    vdisk->free = realloc_as_kernel(vdisk->free, sizeof(uint32_t) * newsize);
    vdisk->used = realloc_as_kernel(vdisk->used, sizeof(uint8_t) * newsize);

    vdisk->sectors = realloc_as_kernel(vdisk->sectors, sizeof(sector_t) * newsize);

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

int vdisk_get_unused_sector(vdisk_t *vdisk) {
    if (vdisk->used_count >= vdisk->size) {
        serial_debug("VDISK", "vdisk full, extending...");
        vdisk_extend(vdisk, vdisk->size + 1000);
    }
    return vdisk->free[vdisk->used_count];
}

int vdisk_write_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data) {
    if (sid.sector >= vdisk->size) {
        sys_error("cannot write sector, out of range");
        return 1;
    }
    mem_copy(vdisk->sectors + sid.sector, data, FS_SECTOR_SIZE);
    return 0;
}

uint8_t *vdisk_load_sector(vdisk_t *vdisk, sid_t sid) {
    if (sid.sector >= vdisk->size) {
        sys_error("cannot load sector, out of range");
        return NULL;
    }
    // return (uint8_t *) (vdisk->sectors + sid.sector);
    uint8_t *data = malloc(FS_SECTOR_SIZE);
    mem_copy(data, vdisk->sectors + sid.sector, FS_SECTOR_SIZE);
    return data;
}

int vdisk_unload_sector(vdisk_t *vdisk, sid_t sid, uint8_t *data, int save) {
    if (save) {
        vdisk_write_sector(vdisk, sid, data);
    }
    free(data);
    return 0;
}
