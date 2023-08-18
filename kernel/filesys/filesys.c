#include <kernel/butterfly.h>
#include <drivers/diskiso.h>
#include <minilib.h>
#include <type.h>


filesys_t *fs_create() {
    filesys_t *filesys = malloc(sizeof(filesys_t));
    filesys->vdisk = malloc(sizeof(vdisk_t *) * FS_DISKS);
    filesys->vdisk_count = 0;
    return filesys;
}

void fs_destroy(filesys_t *filesys) {
    for (uint32_t i = 0; i < FS_DISKS; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        vdisk_destroy(filesys->vdisk[i]);
    }
    free(filesys->vdisk);
    free(filesys);
}

int fs_mount_vdisk(filesys_t *filesys, vdisk_t *vdisk, uint32_t did) {
    if (did > FS_DISKS) {
        kprintf("cannot mount more than %d disks\n", FS_DISKS);
        return -1;
    }
    if (filesys->vdisk[did - 1] != NULL) {
        kprintf("disk %d is already mounted\n", did);
        return -1;
    }
    filesys->vdisk[did - 1] = vdisk;
    filesys->vdisk_count++;
    return did;
}

void fs_print_status(filesys_t *filesys) {
    kprintf("\n====================\n");
    kprintf("vdisk_count: %d\n", filesys->vdisk_count);
    for (uint32_t i = 0; i < FS_DISKS; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        kprintf("vdisk[%d] size: %d, used: %d\n", i,
            filesys->vdisk[i]->size,
            filesys->vdisk[i]->used_count
        );
    }
    kprintf("====================\n\n");
}

filesys_t *MAIN_FS;

filesys_t *fs_get_main() {
    return MAIN_FS;
}

int initrd_to_vdisk(vdisk_t *vdisk) {
    uint8_t *initrd = (uint8_t *) diskiso_get_start();
    uint32_t initrd_size = diskiso_get_size();

    if (initrd_size > vdisk->size * FS_SECTOR_SIZE) {
        kprintf("initrd is too big for the vdisk (initrd: %d, vdisk: %d)\n",
            initrd_size, vdisk->size * FS_SECTOR_SIZE
        );
        return 1;
    }

    if (initrd_size == 0) {
        kprintf("initrd is empty/missing\n");
        return 1;
    }

    for (uint32_t i = 0; i < initrd_size / FS_SECTOR_SIZE; i++) {
        vdisk_write_sector(vdisk, (sid_t) {0, i}, initrd + i * FS_SECTOR_SIZE);
        if (((sector_t*) vdisk->sectors + i)->data[0] &&
            ((sector_t*) vdisk->sectors + i)->data[1]
        ) {
            vdisk_note_sector_used(vdisk, (sid_t) {0, i});
        }
    }

    diskiso_free();

    return 0;
}

int filesys_init() {
    MAIN_FS = fs_create();

    vdisk_t *d0 = vdisk_create(500);
    vdisk_t *d1 = vdisk_create(2000);

    if (initrd_to_vdisk(d1)) {
        return 1;
    }

    fs_mount_vdisk(MAIN_FS, d0, 1);
    fs_mount_vdisk(MAIN_FS, d1, 2);

    fu_dir_create(MAIN_FS, 0, "/");

    fu_dir_create(MAIN_FS, 0, "/tmp");

    fu_add_element_to_dir(
        MAIN_FS,
        ROOT_SID,
        fu_path_to_sid(MAIN_FS, (sid_t) {2, 0}, "/user"),
        "user"
    );

    fu_add_element_to_dir(
        MAIN_FS,
        ROOT_SID,
        fu_path_to_sid(MAIN_FS, (sid_t) {2, 0}, "/bin"),
        "bin"
    );

    fu_add_element_to_dir(
        MAIN_FS,
        ROOT_SID,
        fu_path_to_sid(MAIN_FS, (sid_t) {2, 0}, "/lib"),
        "lib"
    );

    fu_add_element_to_dir(
        MAIN_FS,
        ROOT_SID,
        fu_path_to_sid(MAIN_FS, (sid_t) {2, 0}, "/zada"),
        "zada"
    );

    fs_print_status(MAIN_FS);
    return 0;
}
