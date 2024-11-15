/*****************************************************************************\
|   === main.c : 2024 ===                                                     |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../butterfly.h"

filesys_t *fs_create(void) {
    filesys_t *filesys = malloc(sizeof(filesys_t));
    filesys->vdisk = calloc(FS_DISKS, sizeof(vdisk_t *));
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
        printf("cannot mount more than %d disks\n", FS_DISKS);
        return -1;
    }
    if (filesys->vdisk[did - 1] != NULL) {
        printf("disk %d is already mounted\n", did);
        return -1;
    }
    filesys->vdisk[did - 1] = vdisk;
    filesys->vdisk_count++;
    return did;
}

void fs_print_status(filesys_t *filesys) {
    printf("\n====================\n");
    printf("vdisk_count: %d\n", filesys->vdisk_count);
    for (uint32_t i = 0; i < FS_DISKS; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        printf("vdisk[%d] size: %d, used: %d\n", i,
            filesys->vdisk[i]->size,
            filesys->vdisk[i]->used_count
        );
    }
    printf("====================\n\n");
}

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <input path>\n", argv[0]);
        return 1;
    }

    filesys_t *filesys = fs_create();

    vdisk_t *d0 = vdisk_create(1000);
    fs_mount_vdisk(filesys, d0, 2);
    fs_print_status(filesys);

    fu_dir_create(filesys, 2, "/");

    host_to_internal(filesys, argv[1], "/");
    // internal_to_host(filesys, "output", "/");

    draw_tree(filesys, SID_ROOT, 0);

    fs_print_status(filesys);

    save_vdisk(d0, "initrd.bin");

    fs_destroy(filesys);
    return 0;
}
