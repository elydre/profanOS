#include <kernel/butterfly.h>
#include <minilib.h>
#include <type.h>


sid_t fu_rec_path_to_sid(filesys_t *filesys, sid_t parent, char *path) {
    sid_t ret;

    ret = NULL_SID;

    // read the directory
    uint32_t size = fs_cnt_get_size(filesys, parent);
    if (size == UINT32_MAX) {
        kprintf("failed to get directory size\n");
        return NULL_SID;
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
    sid_t *sids;
    int count;

    count = fu_get_dir_content(filesys, parent, &sids, &names);

    if (count == -1) {
        kprintf("failed to get directory content during path search\n");
        return NULL_SID;
    }

    // search for the path part
    for (int j = 0; j < count; j++) {
        if (str_cmp(path, names[j]) == 0) {
            ret = sids[j];
            break;
        }
        if (str_cmp(name, names[j]) == 0 && fu_is_dir(filesys, sids[j])) {
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

sid_t fu_path_to_sid(filesys_t *filesys, sid_t from, char *path) {
    sid_t ret;

    if (str_cmp("/", path) == 0) {
        ret = ROOT_SID;
    } else if (path[0] == '/') {
        ret = fu_rec_path_to_sid(filesys, from, path + 1);
    } else {
        ret = fu_rec_path_to_sid(filesys, from, path);
    }

    if (IS_NULL_SID(ret)) {
        kprintf("failed to find path %s\n", path);
        return NULL_SID;
    }

    return ret;
}
