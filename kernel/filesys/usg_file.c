/****** This file is part of profanOS **************************\
|   == usg_file.c ==                                 .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel-only file manipulation functions         q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>


int fu_is_file(filesys_t *filesys, sid_t dir_sid) {
    if (IS_NULL_SID(dir_sid)) return 0;
    char *name = fs_cnt_get_meta(filesys, dir_sid);
    if (name == NULL) return 0;
    if (name[0] == 'F') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

sid_t fu_file_create(filesys_t *filesys, int device_id, char *path) {
    char *parent, *name;

    sid_t parent_sid;
    sid_t head_sid;

    sep_path(path, &parent, &name);
    if (!parent[0]) {
        sys_warning("[file_create] Parent unreachable");
        free(parent);
        free(name);
        return NULL_SID;
    }

    parent_sid = fu_path_to_sid(filesys, ROOT_SID, parent);
    if (IS_NULL_SID(parent_sid)) {
        sys_warning("[file_create] Parent not found");
        free(parent);
        free(name);
        return NULL_SID;
    }

    if (!fu_is_dir(filesys, parent_sid)) {
        sys_warning("[file_create] Parent not a directory");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "F-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : parent_sid.device, meta);
    free(meta);

    if (IS_NULL_SID(head_sid)) {
        free(parent);
        free(name);
        return NULL_SID;
    }

    // create a link in parent directory

    if (fu_add_element_to_dir(filesys, parent_sid, head_sid, name)) {
        free(parent);
        free(name);
        return NULL_SID;
    }

    free(parent);
    free(name);

    return head_sid;
}
