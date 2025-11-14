/*****************************************************************************\
|   === filesys.c : 2025 ===                                                  |
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

uint32_t sector_data[SECTOR_SIZE / sizeof(uint32_t)];

/*
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
*/

int filesys_init(void) {
    vdisk_init();

    kfu_dir_create(0, NULL, "/");
    kfu_dir_create(0, "/", "tmp");
    kfu_dir_create(0, "/", "dev");

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
    if (!SID_IS_NULL(src_sid) && kfu_add_element_to_dir(
        SID_ROOT,
        src_sid,
        "src"
    )) return 1;
    return 0;
}
