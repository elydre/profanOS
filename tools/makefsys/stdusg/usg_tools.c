/*****************************************************************************\
|   === fstools.c : 2024 ===                                                  |
|                                                                             |
|    Part of the m creation tool                          .pi0iq.    |
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

void fu_sep_path(const char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = strlen(fullpath);

    if (parent != NULL) {
        *parent = calloc(len + 2, sizeof(char));
    }

    if (cnt != NULL) {
        *cnt = calloc(len + 2, sizeof(char));
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
            strcpy(*parent, "/");
        } else {
            strncpy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        strcpy(*cnt, fullpath + i + 1);
    }
}

void fu_draw_tree(sid_t sid, int depth) {
    sid_t *sids;
    char **names;
    int count;

    count = fu_dir_get_content(sid, &sids, &names);
    
    if (count == 0)
        return;

    if (count == -1) {
        printf("failed to get directory content during path search\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (strcmp(names[i], ".") == 0 || strcmp(names[i], "..") == 0)
            continue;
        
        for (int j = 0; j < depth; j++) 
            printf("  ");
        
        printf("%s, %x: %dB\n", names[i],
                sids[i],
                fs_cnt_get_size(sids[i])
        );
        
        if (fu_is_dir(sids[i]))
            fu_draw_tree(sids[i], depth + 1);
    }

    for (int i = 0; i < count; i++)
        free(names[i]);

    free(names);
    free(sids);
}

void fu_dump_sector(sid_t sid) {
    uint8_t buf[SECTOR_SIZE];
    if (vdisk_read(buf, SECTOR_SIZE, SID_SECTOR(sid) * SECTOR_SIZE)) {
        printf("failed to read sector d%ds%d\n", SID_DISK(sid), SID_SECTOR(sid));
        return;
    }

    printf("SECTOR d%ds%d:\n", SID_DISK(sid), SID_SECTOR(sid));

    char line[16];

    for (int i = 0; i < SECTOR_SIZE; i++) {
        if (i % 16 == 0)
            printf("%04x: ", i);
        printf("%02x ", buf[i]);
        line[i % 16] = (buf[i] >= 32 && buf[i] <= 126) ? buf[i] : '.';
        if (i % 16 == 15) {
            line[16] = 0;
            printf("  %s\n", line);
        }
    }
}
