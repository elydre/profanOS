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

int filesys_init() {
    MAIN_FS = fs_create();

    vdisk_t *d0 = vdisk_create(1000);
    // vdisk_t *d1 = vdisk_create(50);
    // vdisk_t *d1 = load_vdisk("test.bin", 50);
    fs_mount_vdisk(MAIN_FS, d0);
    // fs_mount_vdisk(filesys, d1);

    fs_print_status(MAIN_FS);

    fu_dir_create(MAIN_FS, 0, "/");

    fu_dir_create(MAIN_FS, 0, "/user");
    fu_dir_create(MAIN_FS, 0, "/user/abc");
    fu_dir_create(MAIN_FS, 0, "/user/abc/lalala");
    fu_dir_create(MAIN_FS, 0, "/user/abc/coucou3");
    fu_file_create(MAIN_FS, 0, "/user/abc/coucou3/abc");
    fu_file_create(MAIN_FS, 0, "/user/abc/coucou3/def");

    /* fu_add_element_to_dir(
        filesys,
        fu_path_to_sid(filesys, ROOT_SID, "/user"),
        (sid_t) {2, 0},
        "mount"
    );*/

    draw_tree(MAIN_FS, ROOT_SID, 0);

    fs_print_status(MAIN_FS);
    return 0;
}
