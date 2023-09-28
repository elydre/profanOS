#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <type.h>

#define FILESYS_LIB_C
#include <filesys.h>


/********************************************
 *                                         *
 *          Definitions and macros         *
 *                                         *
********************************************/

sid_t fu_path_to_sid(sid_t from, char *path);

#define CACHE_FCTF_SIZE 16

typedef struct {
    sid_t sid;
    void *addr;
} cache_fctf_t;

cache_fctf_t *cache_fctf;

/********************************************
 *                                         *
 *          Auxiliary functions            *
 *                                         *
********************************************/

// init function
int main(void) {
    cache_fctf = malloc(sizeof(cache_fctf_t) * CACHE_FCTF_SIZE);
    for (int i = 0; i < CACHE_FCTF_SIZE; i++) {
        cache_fctf[i].sid = NULL_SID;
        cache_fctf[i].addr = NULL;
    }

    return 0;
}

void fu_sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    *parent = (char *) malloc(META_MAXLEN);
    *cnt = (char *) malloc(META_MAXLEN);
    (*parent)[0] = '\0';
    (*cnt)[0] = '\0';

    len = strlen(fullpath);

    if (len == 0 || (len == 1 && fullpath[0] == '/')) {
        strcpy((*cnt), "/");
        return;
    }

    if (fullpath[len - 1] == '/') {
        fullpath[len - 1] = '\0';
        len--;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (i <= 0) {
        strcpy(*parent, "/");
        strncpy(*cnt, fullpath + 1 + i, META_MAXLEN);
    } else {
        strncpy(*parent, fullpath, i);
        (*parent)[i] = '\0';
        strncpy(*cnt, fullpath + i + 1, META_MAXLEN);
    }
}

/********************************************
 *                                         *
 *      Standard directory operations      *
 *                                         *
********************************************/

int fu_is_dir(sid_t dir_sid) {
    if (IS_NULL_SID(dir_sid)) return 0;
    filesys_t *filesys = c_fs_get_main();
    char *name = c_fs_cnt_get_meta(filesys, dir_sid);
    if (name == NULL) return 0;
    if (name[0] == 'D') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

/*
DIR STRUCTURE
    [size](4)
    [sid1](8) [name1 offset](4)
    [sid2](8) [name2 offset](4)
    ...
    [sidN](8) [nameN offset](4)
    ...
    [name1](N)
    [name2](N)
    ...
    [nameN](N)
*/

int fu_get_dir_content(sid_t dir_sid, sid_t **ids, char ***names) {
    filesys_t *filesys = c_fs_get_main();

    if (!fu_is_dir(dir_sid)) {
        printf("not a directory\n");
        return -1;
    }

    // read the directory and get size
    uint32_t size = c_fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return -1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (c_fs_cnt_read(filesys, dir_sid, buf, 0, size)) {
        printf("failed to read directory\n");
        return -1;
    }

    // get the number of elements
    uint32_t count;
    memcpy(&count, buf, sizeof(uint32_t));

    if (count == 0 || ids == NULL || names == NULL) {
        free(buf);
        return count;
    }

    // get the elements
    *ids = malloc(sizeof(sid_t) * count);
    *names = malloc(sizeof(char *) * count);

    uint32_t name_offset;
    for (uint32_t i = 0; i < count; i++) {
        memcpy(&(*ids)[i], buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)), sizeof(sid_t));
        memcpy(&name_offset, buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)) + sizeof(sid_t), sizeof(uint32_t));
        char *tmp = (void *) buf + sizeof(uint32_t) + count * (sizeof(sid_t) + sizeof(uint32_t)) + name_offset;
        (*names)[i] = malloc(strlen(tmp) + 1);
        strcpy((*names)[i], tmp);
    }
    free(buf);
    return count;
}

int fu_add_element_to_dir(sid_t dir_sid, sid_t element_sid, char *name) {
    filesys_t *filesys = c_fs_get_main();

    // read the directory and get size
    uint32_t size = c_fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return 1;
    }

    if (!fu_is_dir(dir_sid)) {
        printf("not a directory\n");
        return 1;
    }

    // extend the directory
    if (c_fs_cnt_set_size(filesys, dir_sid, size + sizeof(sid_t) + sizeof(uint32_t) + strlen(name) + 1)) {
        printf("failed to extend directory\n");
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size + sizeof(sid_t) + sizeof(uint32_t) + strlen(name) + 1);
    if (c_fs_cnt_read(filesys, dir_sid, buf, 0, size)) {
        printf("failed to read directory\n");
        return 1;
    }

    uint32_t count = 0;
    // get the number of elements
    memcpy(&count, buf, sizeof(uint32_t));

    if (count > 0) {
        // move names
        for (uint32_t i = size - 1; i >= sizeof(uint32_t) + count * (sizeof(sid_t) + sizeof(uint32_t)); i--) {
            buf[i + sizeof(uint32_t) + sizeof(sid_t)] = buf[i];
        }
    }

    // insert the new element
    memcpy(buf + sizeof(uint32_t) + count * (sizeof(sid_t) + sizeof(uint32_t)), &element_sid, sizeof(sid_t));
    uint32_t name_offset = size - sizeof(uint32_t) - count * (sizeof(sid_t) + sizeof(uint32_t));
    memcpy(buf + sizeof(uint32_t) + count * (sizeof(sid_t) + sizeof(uint32_t)) + sizeof(sid_t), &name_offset, sizeof(uint32_t));

    // update the number of elements
    count++;
    memcpy(buf, &count, sizeof(uint32_t));

    strcpy((char *) buf + sizeof(uint32_t) + count * sizeof(sid_t) + count * sizeof(uint32_t) + name_offset, name);

    // write the directory
    if (c_fs_cnt_write(filesys, dir_sid, buf, 0, size + sizeof(sid_t) + sizeof(uint32_t) + strlen(name) + 1)) {
        printf("failed to write directory\n");
        return 1;
    }

    free(buf);

    return 0;
}

int fu_remove_element_from_dir(sid_t dir_sid, sid_t element_sid) {
    filesys_t *filesys = c_fs_get_main();

    // read the directory and get size
    uint32_t size = c_fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return 1;
    }

    if (!fu_is_dir(dir_sid)) {
        printf("not a directory\n");
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (c_fs_cnt_read(filesys, dir_sid, buf, 0, size)) {
        printf("failed to read directory\n");
        return 1;
    }

    uint32_t count = 0;
    // get the number of elements
    memcpy(&count, buf, sizeof(uint32_t));

    if (count == 0) {
        free(buf);
        return 0;
    }

    // get the elements
    sid_t *ids = malloc(sizeof(sid_t) * count);
    uint32_t *name_offsets = malloc(sizeof(uint32_t) * count);

    for (uint32_t i = 0; i < count; i++) {
        memcpy(&ids[i], buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)), sizeof(sid_t));
        memcpy(&name_offsets[i], buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)) + sizeof(sid_t), sizeof(uint32_t));
    }

    // search for the element
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++) {
        if (IS_SAME_SID(ids[i], element_sid)) {
            index = i + 1;
            break;
        }
    }

    if (index == 0) {
        printf("element not found\n");
        free(name_offsets);
        free(buf);
        free(ids);
        return 1;
    }

    index--;

    for (uint32_t i = index; i < count - 1; i++) {
        ids[i] = ids[i + 1];
        name_offsets[i] = name_offsets[i + 1];
        memcpy(buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)), &ids[i], sizeof(sid_t));
        memcpy(buf + sizeof(uint32_t) + i * (sizeof(sid_t) + sizeof(uint32_t)) + sizeof(sid_t), &name_offsets[i], sizeof(uint32_t));
    }

    // move names
    int name_offset = sizeof(uint32_t) + count * (sizeof(sid_t) + sizeof(uint32_t));
    memmove(buf + name_offset - (sizeof(uint32_t) + sizeof(sid_t)), buf + name_offset, size - name_offset);

    // TODO: remove the name of the removed element

    // update the number of elements
    count--;
    memcpy(buf, &count, sizeof(uint32_t));

    // write the directory
    if (c_fs_cnt_write(filesys, dir_sid, buf, 0,
        size - (sizeof(uint32_t) + sizeof(sid_t)))
    ) {
        printf("failed to write directory\n");
        free(name_offsets);
        free(buf);
        free(ids);
        return 1;
    }

    free(name_offsets);
    free(buf);
    free(ids);

    return 0;
}

sid_t fu_dir_create(int device_id, char *path) {
    filesys_t *filesys = c_fs_get_main();

    char *parent, *name;

    sid_t parent_sid;
    sid_t head_sid;

    // check if the the directory already exists
    head_sid = fu_path_to_sid(ROOT_SID, path);
    if (!IS_NULL_SID(head_sid)) {
        return head_sid;
    }

    fu_sep_path(path, &parent, &name);
    if (parent[0]) {
        parent_sid = fu_path_to_sid(ROOT_SID, parent);
        if (IS_NULL_SID(parent_sid)) {
            printf("failed to find parent directory\n");
            free(parent);
            free(name);
            return NULL_SID;
        }
        if (!fu_is_dir(parent_sid)) {
            printf("parent is not a directory\n");
            free(parent);
            free(name);
            return NULL_SID;
        }
    } else {
        parent_sid.device = 1;
        parent_sid.sector = 0;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    snprintf(meta, META_MAXLEN, "D-%s", name);

    head_sid = c_fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : parent_sid.device, meta);
    free(meta);

    if (IS_NULL_SID(head_sid)) {
        printf("failed to create directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // create a link in parent directory
    if (parent[0]) {
        if (fu_add_element_to_dir(parent_sid, head_sid, name)) {
            printf("failed to add directory to parent\n");
            free(parent);
            free(name);
            return NULL_SID;
        }
    }

    c_fs_cnt_set_size(filesys, head_sid, sizeof(uint32_t));
    c_fs_cnt_write(filesys, head_sid, "\0\0\0\0", 0, 4);

    // create '.' and '..'
    if (fu_add_element_to_dir(head_sid, parent_sid, "..")) {
        printf("failed to add '..' to directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    if (fu_add_element_to_dir(head_sid, head_sid, ".")) {
        printf("failed to add '.' to directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    free(parent);
    free(name);

    return head_sid;
}

/********************************************
 *                                         *
 *        Standard file operations         *
 *                                         *
********************************************/

int fu_is_file(sid_t dir_sid) {
    if (IS_NULL_SID(dir_sid)) return 0;

    filesys_t *filesys = c_fs_get_main();
    char *name = c_fs_cnt_get_meta(filesys, dir_sid);
    if (name == NULL) return 0;
    if (name[0] == 'F') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

sid_t fu_file_create(int device_id, char *path) {
    filesys_t *filesys = c_fs_get_main();

    char *parent, *name;

    sid_t parent_sid;
    sid_t head_sid;

    // check if the the directory already exists
    head_sid = fu_path_to_sid(ROOT_SID, path);
    if (!IS_NULL_SID(head_sid)) {
        return head_sid;
    }

    fu_sep_path(path, &parent, &name);
    if (!parent[0]) {
        printf("parent unreachable\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    parent_sid = fu_path_to_sid(ROOT_SID, parent);
    if (IS_NULL_SID(parent_sid)) {
        printf("failed to find parent directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }
    if (!fu_is_dir(parent_sid)) {
        printf("parent is not a directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    snprintf(meta, META_MAXLEN, "F-%s", name);

    head_sid = c_fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : parent_sid.device, meta);
    free(meta);

    if (IS_NULL_SID(head_sid)) {
        printf("failed to create file\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // create a link in parent directory
    if (fu_add_element_to_dir(parent_sid, head_sid, name)) {
        printf("failed to add directory to parent\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    free(parent);
    free(name);

    return head_sid;
}

int fu_file_read(sid_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || c_fs_cnt_read(c_fs_get_main(), file_sid, buf, offset, size));
}

int fu_file_write(sid_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || c_fs_cnt_write(c_fs_get_main(), file_sid, buf, offset, size));
}

/**************************************************
 *                                               *
 *      Function Calls Through File System       *
 *                                               *
**************************************************/

// call a function when reading a file

int fu_is_fctf(sid_t file_sid) {
    if (IS_NULL_SID(file_sid)) return 0;

    filesys_t *filesys = c_fs_get_main();
    char *name = c_fs_cnt_get_meta(filesys, file_sid);
    if (name == NULL) return 0;
    if (name[0] == 'C') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

sid_t fu_fctf_create(int device_id, char *path, int (*fct)(void *, uint32_t, uint32_t, uint8_t)) {
    filesys_t *filesys = c_fs_get_main();

    char *parent, *name;

    sid_t parent_sid;
    sid_t head_sid;

    // check if the the directory already exists
    head_sid = fu_path_to_sid(ROOT_SID, path);
    if (!IS_NULL_SID(head_sid)) {
        return head_sid;
    }

    fu_sep_path(path, &parent, &name);
    if (!parent[0]) {
        printf("parent unreachable\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    parent_sid = fu_path_to_sid(ROOT_SID, parent);
    if (IS_NULL_SID(parent_sid)) {
        printf("failed to find parent directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }
    if (!fu_is_dir(parent_sid)) {
        printf("parent is not a directory\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    strcpy(meta, "C-");
    strncpy(meta + 2, name, META_MAXLEN - 2);

    head_sid = c_fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : parent_sid.device, meta);
    free(meta);

    if (IS_NULL_SID(head_sid)) {
        printf("failed to create file\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // create a link in parent directory
    if (fu_add_element_to_dir(parent_sid, head_sid, name)) {
        printf("failed to add directory to parent\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    // write the function pointer
    c_fs_cnt_set_size(filesys, head_sid, sizeof(void *));
    if (c_fs_cnt_write(filesys, head_sid, (void *) &fct, 0, sizeof(void *))) {
        printf("failed to write function pointer\n");
        free(parent);
        free(name);
        return NULL_SID;
    }

    free(parent);
    free(name);

    return head_sid;
}

int fu_fctf_rw(sid_t file_sid, void *buf, uint32_t offset, uint32_t size, uint8_t is_read) {
    // search in cache

    void *addr = NULL;

    for (int i = 0; i < CACHE_FCTF_SIZE; i++) {
        if (IS_SAME_SID(cache_fctf[i].sid, file_sid)) {
            addr = cache_fctf[i].addr;
            break;
        }
    }

    if (addr == NULL) {
        // read container
        if (c_fs_cnt_read(c_fs_get_main(), file_sid, &addr, 0, sizeof(void *))) {
            printf("failed to read function pointer\n");
            return 1;
        }

        // add to cache
        for (int i = 0; i < CACHE_FCTF_SIZE; i++) {
            if (IS_NULL_SID(cache_fctf[i].sid)) {
                cache_fctf[i].sid = file_sid;
                cache_fctf[i].addr = addr;
                break;
            }
            if (i == CACHE_FCTF_SIZE - 1) {
                cache_fctf[0].sid = file_sid;
                cache_fctf[0].addr = addr;
            }
        }
    }

    // call the function
    return ((int (*)(void *, uint32_t, uint32_t, uint8_t)) addr)(buf, offset, size, is_read);
}

uint32_t fu_fctf_get_addr(sid_t file_sid) {
    // search in cache

    void *addr = NULL;

    for (int i = 0; i < CACHE_FCTF_SIZE; i++) {
        if (IS_SAME_SID(cache_fctf[i].sid, file_sid)) {
            addr = cache_fctf[i].addr;
            break;
        }
    }

    if (addr == NULL) {
        // read container
        if (c_fs_cnt_read(c_fs_get_main(), file_sid, &addr, 0, sizeof(void *))) {
            printf("failed to read function pointer\n");
            return 1;
        }

        // add to cache
        for (int i = 0; i < CACHE_FCTF_SIZE; i++) {
            if (IS_NULL_SID(cache_fctf[i].sid)) {
                cache_fctf[i].sid = file_sid;
                cache_fctf[i].addr = addr;
                break;
            }
            if (i == CACHE_FCTF_SIZE - 1) {
                cache_fctf[0].sid = file_sid;
                cache_fctf[0].addr = addr;
            }
        }
    }

    return (uint32_t) addr;
}

/**************************************************
 *                                               *
 *            Path to SID conversion             *
 *                                               *
**************************************************/

sid_t fu_rec_path_to_sid(filesys_t *filesys, sid_t parent, char *path) {
    sid_t ret;

    ret = NULL_SID;

    // read the directory
    uint32_t size = c_fs_cnt_get_size(filesys, parent);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return NULL_SID;
    }

    // generate the path part to search for
    char *name;

    name = calloc(1, strlen(path) + 1);
    strcpy(name, path);
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

    count = fu_get_dir_content(parent, &sids, &names);

    if (count == -1) {
        printf("failed to get directory content during path search\n");
        return NULL_SID;
    }

    // search for the path part
    for (int j = 0; j < count; j++) {
        if (strcmp(path, names[j]) == 0) {
            ret = sids[j];
            break;
        }
        if (strcmp(name, names[j]) == 0 && fu_is_dir(sids[j]) && fu_get_dir_content(sids[j], NULL, NULL) > 0) {
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

sid_t fu_path_to_sid(sid_t from, char *path) {
    filesys_t *filesys = c_fs_get_main();

    sid_t ret;

    if (strcmp("/", path) == 0) {
        return ROOT_SID;
    }

    int len = strlen(path) - 1;
    char *tmp;

    if (path[len] == '/') {
        tmp = strdup(path);
        tmp[len] = '\0';
    } else {
        tmp = path;
    }

    if (tmp[0] == '/') {
        ret = fu_rec_path_to_sid(filesys, from, tmp + 1);
    } else {
        ret = fu_rec_path_to_sid(filesys, from, tmp);
    }

    if (tmp != path) {
        free(tmp);
    }

    return ret;
}

void fu_simplify_path(char *path) {
    // some path look like this: /a/b/../c/./d/./e/../f
    // this function simplifies them to: /a/c/d/f

    if (path[0] != '/') {
        printf("Cannot simplify relative path\n");
        return;
    }

    char *tmp = malloc(strlen(path) + 2);
    strcpy(tmp, path);
    strcat(tmp, "/");

    int i = 0;
    int k = 0;

    while (tmp[i]) {
        if (tmp[i] == '.' && tmp[i + 1] == '.') {
            i += 2;
            for (k -= 2; k >= 0 && path[k] != '/'; k--);
            continue;
        }
        if (tmp[i] == '.' && tmp[i + 1] == '/') {
            i += 2;
            continue;
        }
        path[k] = tmp[i];
        i++;
        k++;
    }

    if (k > 1 && path[k - 1] == '/') {
        path[k - 1] = '\0';
    } else {
        path[k] = '\0';
    }

    // remove successive slashes
    i = 1;
    while (path[i]) {
        if (path[i] == '/' && path[i - 1] == '/') {
            for (k = i; path[k]; k++) {
                path[k] = path[k + 1];
            }
            continue;
        }
        i++;
    }

    if (i > 1 && path[i - 1] == '/' && path[i - 2] != '/') {
        path[i - 1] = '\0';
    }

    free(tmp);
}

/**************************************************
 *                                               *
 *             File System Get Info              *
 *                                               *
**************************************************/

uint32_t *fu_get_vdisk_info() {
    // array:
    // [0] = vdisk_count
    // [3n + 1] = vdisk[n] mount point
    // [3n + 2] = vdisk[n] used_count
    // [3n + 3] = vdisk[n] size
    // ...

    filesys_t *filesys = c_fs_get_main();

    uint32_t *ret = malloc(sizeof(uint32_t) * (filesys->vdisk_count * 3 + 1));
    ret[0] = filesys->vdisk_count;
    int ret_i = 1;
    for (uint32_t i = 0; i < filesys->max_disks; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        ret[ret_i] = i + 1;
        ret[ret_i + 1] = filesys->vdisk[i]->used_count;
        ret[ret_i + 2] = filesys->vdisk[i]->size;
        ret_i += 3;
    }

    return ret;
}
