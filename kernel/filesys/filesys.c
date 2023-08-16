#include <butterfly.h>
#include <minilib.h>


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
        printf("cannot mount more than %d disks\n", FS_DISKS);
        return -1;
    }
    filesys->vdisk[did] = vdisk;
    filesys->vdisk_count++;
    return did + 1;
}

void fs_print_status(filesys_t *filesys) {
    printf("\n====================\n");
    printf("vdisk_count: %d\n", filesys->vdisk_count);
    for (uint32_t i = 0; i < filesys->vdisk_count; i++) {
        printf("vdisk[%d] size: %d, used: %d\n", i,
            filesys->vdisk[i]->size,
            filesys->vdisk[i]->used_count
        );
    }
    printf("====================\n\n");
}

int main(void) {
    filesys_t *filesys = fs_create();

    vdisk_t *d0 = vdisk_create(10000000);
    // vdisk_t *d1 = vdisk_create(50);
    // vdisk_t *d1 = load_vdisk("test.bin", 50);
    fs_mount_vdisk(filesys, d0);
    // fs_mount_vdisk(filesys, d1);

    fs_print_status(filesys);

    fu_dir_create(filesys, 0, "/");

    /* fu_dir_create(filesys, 2, "/user/abc");
    fu_dir_create(filesys, 0, "/user/abc/lalala");
    fu_dir_create(filesys, 0, "/user/abc/coucou3");
    fu_file_create(filesys, 0, "/user/abc/coucou3/abc");
    fu_file_create(filesys, 0, "/user/abc/coucou3/def"); */

    /* fu_add_element_to_dir(
        filesys,
        fu_path_to_sid(filesys, ROOT_SID, "/user"),
        (sid_t) {2, 0},
        "mount"
    );*/

    host_to_internal(filesys, "input", "/");
    internal_to_host(filesys, "output", "/");

    draw_tree(filesys, ROOT_SID, 0);

    fs_print_status(filesys);

    // save_vdisk(d1, "test.bin");

    fs_destroy(filesys);
    return 0;
}
