/*****************************************************************************\
|   === usg_ptsid.c : 2024 ===                                                |
|                                                                             |
|    Kernel path resolution functions                              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>

uint32_t kfu_rec_path_to_sid(uint32_t parent, const char *path) {
    uint32_t ret;

    ret = SID_NULL;

    // read the directory
    uint32_t size = fs_cnt_get_size(parent);
    if (size == UINT32_MAX) {
        return SID_NULL;
    }

    // generate the path part to search for
    char *name;

    name = calloc(str_len(path) + 1);
    str_cpy(name, path);
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

    count = kfu_get_dir_content(parent, &sids, &names);

    if (count < 1) {
        free(name);
        return SID_NULL;
    }

    // search for the path part
    for (int j = 0; j < count; j++) {
        if (str_cmp(path, names[j]) == 0) {
            ret = sids[j];
            break;
        }
        if (str_cmp(name, names[j]) == 0 && kfu_is_dir(sids[j]) &&
            kfu_get_dir_content(sids[j], NULL, NULL) > 0
        ) {
            ret = kfu_rec_path_to_sid(sids[j], path + i);
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

uint32_t kfu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (str_cmp("/", path) == 0) {
        ret = SID_ROOT;
    } else if (path[0] == '/') {
        ret = kfu_rec_path_to_sid(from, path + 1);
    } else {
        ret = kfu_rec_path_to_sid(from, path);
    }

    return ret;
}
