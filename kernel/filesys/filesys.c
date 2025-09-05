/*****************************************************************************\
|   === filesys.c : 2024 ===                                                  |
|                                                                             |
|    Kernel filesystem functions                                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <drivers/diskiso.h>
#include <minilib.h>
#include <system.h>

filesys_t *MAIN_FS;

filesys_t *fs_create(void) {
    filesys_t *filesys = malloc(sizeof(filesys_t));
    filesys->vdisk = calloc(sizeof(vdisk_t *) * FS_MAX_DISKS);
    filesys->vdisk_count = 0;
    return filesys;
}

void fs_destroy(filesys_t *filesys) {
    for (uint32_t i = 0; i < FS_MAX_DISKS; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        vdisk_destroy(filesys->vdisk[i]);
    }
    free(filesys->vdisk);
    free(filesys);
}

int fs_mount_vdisk(vdisk_t *vdisk, uint8_t device_id) {
    if (MAIN_FS->vdisk[device_id - 1] != NULL) {
        sys_warning("[mount_vdisk] Disk id is already used");
        return -1;
    }
    MAIN_FS->vdisk[device_id - 1] = vdisk;
    MAIN_FS->vdisk_count++;
    return device_id;
}

filesys_t *fs_get_filesys(void) {
    return MAIN_FS;
}

vdisk_t *initrd_to_vdisk(void) {
    uint8_t *initrd = diskiso_get_start();
    uint32_t initrd_size = diskiso_get_size();

    if (initrd_size == 0) {
        sys_error("Initrd is empty or missing");
        return NULL;
    }

    vdisk_t *vdisk = vdisk_create(initrd_size / FS_SECTOR_SIZE + 1);

    for (uint32_t i = 0; i < initrd_size / FS_SECTOR_SIZE; i++) {
        vdisk_write_sector(vdisk, SID_FORMAT(0, i), initrd + i * FS_SECTOR_SIZE);
        if (((sector_t*) vdisk->sectors + i)->data[0]) {
            vdisk_note_sector_used(vdisk, SID_FORMAT(0, i));
        }
    }

    diskiso_free();

    return vdisk;
}

int filesys_init(void) {
    MAIN_FS = fs_create();

    vdisk_t *d0 = vdisk_create(500);
    if (d0 == NULL)
        return 1;

    fs_mount_vdisk(d0, 1);

    kfu_dir_create(0, NULL, "/");
    kfu_dir_create(0, "/", "tmp");
    kfu_dir_create(0, "/", "dev");

    vdisk_t *d1 = initrd_to_vdisk();
    if (d1 == NULL)
        return 1;

    fs_mount_vdisk(d1, 2);

    if (kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(2, 0), "/user"),
        "user"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(2, 0), "/bin"),
        "bin"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(2, 0), "/lib"),
        "lib"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(2, 0), "/sys"),
        "sys"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(2, 0), "/zada"),
        "zada"
    )) {
        return 1;
    }

    uint32_t src_sid = kfu_path_to_sid(SID_FORMAT(2, 0), "/src");
    if (!IS_SID_NULL(src_sid) && kfu_add_element_to_dir(
        SID_ROOT,
        src_sid,
        "src"
    )) return 1;
    return 0;
}
