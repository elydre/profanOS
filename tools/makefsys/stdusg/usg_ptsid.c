/*****************************************************************************\
|   === usg_ptsid.c : 2024 ===                                                |
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

uint32_t fu_rec_path_to_sid(filesys_t *filesys, uint32_t parent, char *path) {
    uint32_t ret;

    ret = SID_NULL;

    // read the directory
    uint32_t size = fs_cnt_get_size(filesys, parent);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return SID_NULL;
    }

    // generate the path part to search for
    char *name;

    name = calloc(1, strlen(path) + 1);
    strcpy(name, path);
    uint32_t i = 0;
    while (name[i]) {
        if (name[i] == '/') {
            name[i] = '\0';
            break;
        }
        i++;
    }
    while (path[i] == '/') i++;

    // get the directory content
    char **names;
    uint32_t *sids;
    int count;

    count = fu_get_dir_content(filesys, parent, &sids, &names);

    if (count == -1) {
        printf("failed to get directory content during path search\n");
        free(name);
        return SID_NULL;
    }

    // search for the path part
    for (int j = 0; j < count; j++) {
        if (strcmp(path, names[j]) == 0) {
            ret = sids[j];
            break;
        }
        if (strcmp(name, names[j]) == 0 && fu_is_dir(filesys, sids[j]) &&
            fu_get_dir_content(filesys, sids[j], NULL, NULL) > 0
        ) {
            ret = fu_rec_path_to_sid(filesys, sids[j], path + i);
            break;
        }
    }

    // free
    for (int j = 0; j < count; j++) {
        free(names[j]);
    }
    free(names);
    free(sids);
    free(name);

    return ret;
}

uint32_t fu_path_to_sid(filesys_t *filesys, uint32_t from, char *path) {
    uint32_t ret;

    if (strcmp("/", path) == 0) {
        ret = SID_ROOT;
    } else if (path[0] == '/') {
        ret = fu_rec_path_to_sid(filesys, from, path + 1);
    } else {
        ret = fu_rec_path_to_sid(filesys, from, path);
    }

    if (IS_SID_NULL(ret)) {
        printf("failed to find path %s\n", path);
        return SID_NULL;
    }

    return ret;
}
