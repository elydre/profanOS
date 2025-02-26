/*****************************************************************************\
|   === usg_file.c : 2024 ===                                                 |
|                                                                             |
|    Kernel-only file manipulation functions                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fu_is_file(filesys_t *filesys, uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid))
        return 0;

    char *name = fs_cnt_meta(filesys, dir_sid, NULL);
    if (name == NULL)
        return 0;

    if (name[0] == 'F') {
        free(name);
        return 1;
    }

    free(name);
    return 0;
}

uint32_t fu_file_create(filesys_t *filesys, char *parent, char *name) {
    uint32_t parent_sid;
    uint32_t head_sid;

    parent_sid = fu_path_to_sid(filesys, SID_ROOT, parent);

    if (!fu_is_dir(filesys, parent_sid)) {
        sys_warning("[file_create] Parent unreachable");
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "F-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(filesys, SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid))
        return SID_NULL;

    // create a link in parent directory
    return fu_add_element_to_dir(filesys, parent_sid, head_sid, name) ? SID_NULL : head_sid;
}

uint32_t fu_afft_create(filesys_t *filesys, char *parent, char *name, uint32_t id) {
    uint32_t parent_sid;
    uint32_t head_sid;

    parent_sid = fu_path_to_sid(filesys, SID_ROOT, parent);

    if (!fu_is_dir(filesys, parent_sid)) {
        sys_warning("[file_create] Parent unreachable");
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "A-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(filesys, SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid) || fu_add_element_to_dir(filesys, parent_sid, head_sid, name))
        return SID_NULL;

    // write the id
    return (fs_cnt_set_size(filesys, head_sid, sizeof(uint32_t)) ||
            fs_cnt_write(filesys, head_sid, &id, 0, sizeof(uint32_t)) ? SID_NULL : head_sid);
}
