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

static inline int path_to_sid_cmp(const char *path, const char *name, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        if (name[i] == '\0' || path[i] != name[i])
            return 1;
    }

    return name[len] != '\0';
}

static uint32_t rec_path_to_sid(uint32_t parent, const char *path) {
    // generate the path part to search for
    uint32_t path_len = 0;

    while (*path == '/')
        path++;

    while (path[path_len] && path[path_len] != '/')
        path_len++;

    if (path_len == 0)
        return parent;

    // get the directory content
    uint32_t sid, size;
    uint8_t *buf;
    int offset;

    size = fs_cnt_get_size(parent);

    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return SID_NULL;

    buf = malloc(size);

    if (fs_cnt_read(parent, buf, 0, size)) {
        free(buf);
        return SID_NULL;
    }

    // search for the path part
    for (int j = 0; (offset = fu_dir_get_elm(buf, size, j, &sid)) > 0; j++) {
        if (strcmp(path, (char *) buf + offset) == 0) {
            free(buf);
            return sid;
        }

        if (path_to_sid_cmp(path, (char *) buf + offset, path_len) == 0 && fu_is_dir(sid)) {
            free(buf);
            return rec_path_to_sid(sid, path + path_len);
        }
    }

    free(buf);

    return SID_NULL;
}

uint32_t fu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (strcmp("/", path) == 0)
        return from;

    ret = rec_path_to_sid(from, path);

    return ret;
}
