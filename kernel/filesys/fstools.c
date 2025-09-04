/*****************************************************************************\
|   === fstools.c : 2024 ===                                                  |
|                                                                             |
|    Kernel filesystem tools                                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>

void kfu_sep_path(const char *fullpath, char **parent, char **cnt) {
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

vdisk_t *fs_get_vdisk(uint8_t device_id) {
    device_id -= 1;
    return MAIN_FS->vdisk[device_id];
}
