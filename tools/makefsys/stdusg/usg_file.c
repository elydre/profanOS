/*****************************************************************************\
|   === usg_file.c : 2024 ===                                                 |
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

int fu_is_file(filesys_t *filesys, uint32_t dir_sid) {
    char *name = fs_cnt_get_meta(filesys, dir_sid);
    if (name == NULL) return 0;
    if (name[0] == 'F') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

uint32_t fu_file_create(filesys_t *filesys, int device_id, char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    sep_path(path, &parent, &name);
    if (!parent[0]) {
        printf("parent unreachable\n");
        free(parent);
        free(name);
        return SID_NULL;
    }

    parent_sid = fu_path_to_sid(filesys, SID_ROOT, parent);
    if (IS_SID_NULL(parent_sid)) {
        printf("failed to find parent directory\n");
        free(parent);
        free(name);
        return SID_NULL;
    }
    if (!fu_is_dir(filesys, parent_sid)) {
        printf("parent is not a directory\n");
        free(parent);
        free(name);
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    snprintf(meta, META_MAXLEN, "F-%s", name);

    head_sid = fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        printf("failed to create file\n");
        free(parent);
        free(name);
        return SID_NULL;
    }

    // create a link in parent directory

    if (fu_add_element_to_dir(filesys, parent_sid, head_sid, name)) {
        printf("failed to add directory to parent\n");
        free(parent);
        free(name);
        return SID_NULL;
    }

    free(parent);
    free(name);

    return head_sid;
}
