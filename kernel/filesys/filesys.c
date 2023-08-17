#include <kernel/butterfly.h>
#include <minilib.h>
#include <type.h>


filesys_t *fs_create() {
    filesys_t *filesys = malloc(sizeof(filesys_t));
    filesys->vdisk = malloc(sizeof(vdisk_t *) * FS_DISKS);
    filesys->vdisk_count = 0;
    return filesys;
}

void fs_destroy(filesys_t *filesys) {
    for (uint32_t i = 0; i < filesys->vdisk_count; i++) {
        vdisk_destroy(filesys->vdisk[i]);
    }
    free(filesys->vdisk);
    free(filesys);
}

int fs_mount_vdisk(filesys_t *filesys, vdisk_t *vdisk) {
    uint32_t did;
    for (did = 0; did < filesys->vdisk_count; did++) {
        if (filesys->vdisk[did] == NULL) {
            break;
        }
    }
    if (did == FS_DISKS) {
        kprintf("cannot mount more than %d disks\n", FS_DISKS);
        return -1;
    }
    filesys->vdisk[did] = vdisk;
    filesys->vdisk_count++;
    return did + 1;
}

void fs_print_status(filesys_t *filesys) {
    kprintf("\n====================\n");
    kprintf("vdisk_count: %d\n", filesys->vdisk_count);
    for (uint32_t i = 0; i < filesys->vdisk_count; i++) {
        kprintf("vdisk[%d] size: %d, used: %d\n", i,
            filesys->vdisk[i]->size,
            filesys->vdisk[i]->used_count
        );
    }
    kprintf("====================\n\n");
}

filesys_t *MAIN_FS;

filesys_t *get_main_fs() {
    return MAIN_FS;
}

int initrd_to_vdisk(vdisk_t *vdisk) {
    uint8_t *initrd = (uint8_t *) diskiso_get_start();
    uint32_t initrd_size = diskiso_get_size();

    if (initrd_size > vdisk->size) {
        kprintf("initrd is too big for the vdisk\n");
        return 1;
    }

    for (uint32_t i = 0; i < initrd_size / SECTOR_SIZE; i++) {
        vdisk_write_sector(vdisk, (sid_t) {0, i}, initrd + i * SECTOR_SIZE);
    }

    diskiso_free();

    return 0;
}

int filesys_init() {
    MAIN_FS = fs_create();

    vdisk_t *d0 = vdisk_create(10000);
    vdisk_t *d1 = vdisk_create(10000);
    initrd_to_vdisk(d1);
    fs_mount_vdisk(MAIN_FS, d0);
    fs_mount_vdisk(MAIN_FS, d1);

    fs_print_status(MAIN_FS);

    fu_dir_create(MAIN_FS, 0, "/");

    fu_dir_create(MAIN_FS, 0, "/tmp");

    fu_add_element_to_dir(
        MAIN_FS,
        ROOT_SID,
        fu_path_to_sid(MAIN_FS, (sid_t) {2, 0}, "/user"),
        "mount"
    );

    draw_tree(MAIN_FS, ROOT_SID, 0);

    fs_print_status(MAIN_FS);
    return 0;
}
