/****** This file is part of profanOS **************************\
|   == fstools.c ==                                  .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel filesystem tools                         q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>


void sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = str_len(fullpath);

    if (parent != NULL) {
        *parent = calloc(len + 2);
    }

    if (cnt != NULL) {
        *cnt = calloc(len + 2);
    }

    while (len > 0 && fullpath[len - 1] == '/') {
        len--;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (parent != NULL && i >= 0) {
        if (i == 0) {
            str_cpy(*parent, "/");
        } else {
            str_ncpy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        str_cpy(*cnt, fullpath + i + 1);
    }
}


vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id) {
    device_id -= 1;
    if (device_id >= fs->max_disks || fs->vdisk[device_id] == NULL) {
        return NULL;
    }
    return fs->vdisk[device_id];
}
