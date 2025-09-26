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

int fu_is_file(sid_t dir_sid) {
    char letter;

    if (fs_cnt_meta(dir_sid, &letter, 1, 0))
        return 0;

    return letter == 'F';
}

sid_t fu_file_create(const char *parent, const char *name) {
    sid_t parent_sid;
    sid_t head_sid;

    parent_sid = fu_path_to_sid(SID_ROOT, parent);

    if (!fu_is_dir(parent_sid)) {
        printf("Error: parent is not a directory\n");
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    strcpy(meta, "F-");
    strncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid))
        return SID_NULL;

    // create a link in parent directory
    return fu_add_element_to_dir(parent_sid, head_sid, name) ? SID_NULL : head_sid;
}
