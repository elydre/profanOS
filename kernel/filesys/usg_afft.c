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

int kfu_is_afft(uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid))
        return 0;

    char *name = fs_cnt_meta(dir_sid, NULL);
    if (name == NULL)
        return 0;

    if (name[0] == 'A') {
        free(name);
        return 1;
    }

    free(name);
    return 0;
}

int kfu_afft_get_id(uint32_t sid) {
    uint32_t id;

    if (!kfu_is_afft(sid))
        return -1;

    if (fs_cnt_get_size(sid) != sizeof(uint32_t) || fs_cnt_read(sid, &id, 0, sizeof(uint32_t)))
        return -1;

    return id;
}

uint32_t kfu_afft_create(const char *parent, const char *name, uint32_t id) {
    uint32_t parent_sid;
    uint32_t head_sid;

    parent_sid = kfu_path_to_sid(SID_ROOT, parent);

    if (!kfu_is_dir(parent_sid)) {
        sys_warning("[file_create] Parent unreachable");
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "A-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid) || kfu_add_element_to_dir(parent_sid, head_sid, name))
        return SID_NULL;

    // write the id
    return (fs_cnt_set_size(head_sid, sizeof(uint32_t)) ||
            fs_cnt_write(head_sid, &id, 0, sizeof(uint32_t)) ? SID_NULL : head_sid);
}
