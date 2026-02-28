/*****************************************************************************\
|   === usg_file.c : 2025 ===                                                 |
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

int kfu_is_file(sid_t sid) {
    char letter;

    if (SID_IS_NULL(sid))
        return 0;

    if (fs_cnt_meta(sid, &letter, 1, 0))
        return 0;

    return letter == 'F';
}

sid_t kfu_file_create(const char *parent, const char *name) {
    sid_t parent_sid;
    sid_t head_sid;

    parent_sid = kfu_path_to_sid(SID_ROOT, parent);

    if (!kfu_is_dir(parent_sid)) {
        sys_warning("[file_create] Parent unreachable");
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_copy(meta, "F-");
    str_ncopy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(SID_DISK(parent_sid), meta);
    free(meta);

    if (SID_IS_NULL(head_sid))
        return SID_NULL;

    // create a link in parent directory
    return kfu_add_element_to_dir(parent_sid, head_sid, name) ? SID_NULL : head_sid;
}
