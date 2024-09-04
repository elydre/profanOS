/*****************************************************************************\
|   === fstools.c : 2024 ===                                                  |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include "../butterfly.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

void sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    *parent = (char *)malloc(META_MAXLEN);
    *cnt = (char *)malloc(META_MAXLEN);

    len = strlen(fullpath);
    if (len == 0 || (len == 1 && fullpath[0] == '/')) {
        (*parent)[0] = '\0';
        strcpy((*cnt), "/");
        return;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (i <= 0) {
        strcpy(*parent, "/");
        strncpy(*cnt, fullpath + 1 + i, META_MAXLEN);
    } else {
        strncpy(*parent, fullpath, i);
        (*parent)[i] = '\0';
        strncpy(*cnt, fullpath + i + 1, META_MAXLEN);
    }
}

vdisk_t *fs_get_vdisk(filesys_t *fs, uint32_t device_id) {
    device_id -= 1;
    if (device_id >= FS_DISKS || fs->vdisk[device_id] == NULL) {
        return NULL;
    }
    return fs->vdisk[device_id];
}

void draw_tree(filesys_t *filesys, uint32_t sid, int depth) {
    char **names;
    uint32_t *sids;
    int count;

    count = fu_get_dir_content(filesys, sid, &sids, &names);

    if (count == -1) {
        printf("failed to get directory content during path search\n");
        return;
    }

    if (count == 0) {
        return;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0) {
            continue;
        }
        for (int j = 0; j < depth; j++) {
            printf("  ");
        }
        printf("%s, d%ds%d: %dB\n", names[i],
                SID_DISK(sids[i]),
                SID_SECTOR(sids[i]),
                fs_cnt_get_size(filesys, sids[i])
        );
        if (fu_is_dir(filesys, sids[i])) {
            draw_tree(filesys, sids[i], depth + 1);
        }
    }

    for (int i = 0; i < count; i++) {
        free(names[i]);
    }
    free(names);
    free(sids);
}
