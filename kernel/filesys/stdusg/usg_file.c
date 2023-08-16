#include <butterfly.h>
#include <minilib.h>


int fu_is_file(filesys_t *filesys, sid_t dir_sid) {
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
        printf("parent unreachable\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    parent_sid = fu_path_to_sid(filesys, ROOT_SID, parent);
    if (IS_NULL_SID(parent_sid)) {
        printf("failed to find parent directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }
    if (!fu_is_dir(filesys, parent_sid)) {
        printf("parent is not a directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    snprintf(meta, META_MAXLEN, "F-%s", name);

    head_sid = fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : parent_sid.device, meta);
    free(meta);

    if (IS_NULL_SID(head_sid)) {
        printf("failed to create file\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // create a link in parent directory

    if (fu_add_element_to_dir(filesys, parent_sid, head_sid, name)) {
        printf("failed to add directory to parent\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    free(parent);
    free(name);

    return head_sid;
}
