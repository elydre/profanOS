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

uint32_t sector_data[SECTOR_SIZE / sizeof(uint32_t)];

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("usage: %s <input path>\n", argv[0]);
        return 1;
    }

    vdisk_init();

    fu_dir_create(0, "/");

    hio_dir_import(argv[1], "/");
    // internal_to_host(filesys, "output", "/");

    fu_draw_tree(SID_ROOT, 0);

    hio_raw_export("initrd.bin");

    vdisk_destroy();

    return 0;
}
