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

int initrd_to_vdisk(void) {
    uint8_t *initrd = diskiso_get_start();
    uint32_t initrd_size = diskiso_get_size();

    if (initrd_size == 0) {
        sys_error("Initrd is empty or missing");
        return 1;
    }

    int afft = vdisk_diskify(initrd, initrd_size);

    if (afft < 0 || interdisk_register_disk(1, afft)) {
        sys_error("Failed to create vdisk for initrd");
        return 1;
    }

    diskiso_fini();
    return 0;
}

int filesys_init(void) {
    interdisk_init();
    vdisk_init();

    int main_disk = vdisk_create();
    if (main_disk < 0 || interdisk_register_disk(0, main_disk)) {
        sys_error("Failed to create vdisk for filesystem");
        return 1;
    }

    kfu_dir_create(0, NULL, "/");
    kfu_dir_create(0, "/", "tmp");
    kfu_dir_create(0, "/", "dev");

    if (initrd_to_vdisk())
        return 1;

    if (kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(1, 1), "/user"),
        "user"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(1, 1), "/bin"),
        "bin"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(1, 1), "/lib"),
        "lib"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(1, 1), "/sys"),
        "sys"
    ) || kfu_add_element_to_dir(
        SID_ROOT,
        kfu_path_to_sid(SID_FORMAT(1, 1), "/zada"),
        "zada"
    )) {
        return 1;
    }

    uint32_t src_sid = kfu_path_to_sid(SID_FORMAT(1, 1), "/src");

    return (!SID_IS_NULL(src_sid) && kfu_add_element_to_dir(
        SID_ROOT,
        src_sid,
        "src"
    ));
}
