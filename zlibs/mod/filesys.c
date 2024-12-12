/*****************************************************************************\
|   === filesys.c : 2024 ===                                                  |
|                                                                             |
|    File System extension module (wiki/lib_filesys)               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_STATIC
#include <profan/syscall.h>

#include <profan/libmmq.h>
#include <profan/types.h>

#define FILESYS_LIB_C
#include <profan/filesys.h>


/********************************************
 *                                         *
 *          Definitions and macros         *
 *                                         *
********************************************/

uint32_t fu_path_to_sid(uint32_t from, const char *path);

#define CACHE_FCTF_SIZE 16
#define RAISE_ERROR(...) fd_printf(2, "[FS MODULE ERROR] " __VA_ARGS__)

/********************************************
 *                                         *
 *          Auxiliary functions            *
 *                                         *
********************************************/

// init function
int main(void) {
    return 0;
}

static void fu_sep_path(const char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = strlen(fullpath);

    if (parent != NULL) {
        *parent = calloc(1, len + 2);
    }

    if (cnt != NULL) {
        *cnt = calloc(1, len + 2);
    }

    while (len > 0 && fullpath[len - 1] == '/') {
        len--;
    }

    for (i = len - 1; i >= 0; i--) {
        if (fullpath[i] == '/') {
            break;
        }
    }

    if (parent != NULL && i >= 0) {
        if (i == 0) {
            strcpy(*parent, "/");
        } else {
            strncpy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        strcpy(*cnt, fullpath + i + 1);
    }
}

/********************************************
 *                                         *
 *      Standard directory operations      *
 *                                         *
********************************************/

int fu_is_dir(uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid)) return 0;
    char *name = syscall_fs_meta(NULL, dir_sid, NULL);
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

int fu_get_dir_content(uint32_t dir_sid, uint32_t **ids, char ***names) {
    if (!fu_is_dir(dir_sid)) {
        RAISE_ERROR("get_dir_content: d%ds%d is not a directory\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return -1;
    }

    // read the directory and get size
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX) {
        RAISE_ERROR("get_dir_content: failed to get size of d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return -1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size)) {
        RAISE_ERROR("get_dir_content: failed to read from d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
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
    *ids = malloc(sizeof(uint32_t) * count);
    *names = malloc(sizeof(char *) * count);

    uint32_t name_offset;
    for (uint32_t i = 0; i < count; i++) {
        memcpy(&(*ids)[i], buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)), sizeof(uint32_t));
        memcpy(&name_offset,
                buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t),
                sizeof(uint32_t)
        );
        char *tmp = (void *) buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)) + name_offset;
        (*names)[i] = malloc(strlen(tmp) + 1);
        strcpy((*names)[i], tmp);
    }
    free(buf);
    return count;
}

int fu_add_to_dir(uint32_t dir_sid, uint32_t element_sid, char *name) {
    // read the directory and get size
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX) {
        RAISE_ERROR("add_element_to_dir: failed to get size of d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    if (!fu_is_dir(dir_sid)) {
        RAISE_ERROR("add_element_to_dir: d%ds%d is not a directory\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    // extend the directory
    if (syscall_fs_set_size(NULL, dir_sid, size + sizeof(uint32_t) + sizeof(uint32_t) + strlen(name) + 1)) {
        RAISE_ERROR("add_element_to_dir: failed to extend d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size + sizeof(uint32_t) + sizeof(uint32_t) + strlen(name) + 1);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size)) {
        RAISE_ERROR("add_element_to_dir: failed to read from d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    uint32_t count = 0;
    // get the number of elements
    memcpy(&count, buf, sizeof(uint32_t));

    if (count > 0) {
        // move names
        for (uint32_t i = size - 1; i >= sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)); i--) {
            buf[i + sizeof(uint32_t) + sizeof(uint32_t)] = buf[i];
        }
    }

    // insert the new element
    memcpy(buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)), &element_sid, sizeof(uint32_t));
    uint32_t name_offset = size - sizeof(uint32_t) - count * (sizeof(uint32_t) + sizeof(uint32_t));

    memcpy(buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t),
            &name_offset, sizeof(uint32_t));

    // update the number of elements
    count++;
    memcpy(buf, &count, sizeof(uint32_t));

    strcpy((char *) buf + sizeof(uint32_t) + count * sizeof(uint32_t) + count * sizeof(uint32_t) + name_offset, name);

    // write the directory
    if (syscall_fs_write(NULL, dir_sid, buf, 0, size + sizeof(uint32_t) + sizeof(uint32_t) + strlen(name) + 1)) {
        RAISE_ERROR("add_element_to_dir: failed to write to d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    free(buf);

    return 0;
}

int fu_remove_from_dir(uint32_t dir_sid, uint32_t element_sid) {
    // read the directory and get size
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX) {
        RAISE_ERROR("remove_element_from_dir: failed to get size of d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    if (!fu_is_dir(dir_sid)) {
        RAISE_ERROR("remove_element_from_dir: d%ds%d is not a directory\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size)) {
        RAISE_ERROR("remove_element_from_dir: failed to read from d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
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
    uint32_t *ids = malloc(sizeof(uint32_t) * count);
    uint32_t *name_offsets = malloc(sizeof(uint32_t) * count);

    for (uint32_t i = 0; i < count; i++) {
        memcpy(&ids[i], buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)), sizeof(uint32_t));
        memcpy(&name_offsets[i],
                buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t),
                sizeof(uint32_t)
        );
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
        RAISE_ERROR("remove_element_from_dir: element not found in d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
        free(name_offsets);
        free(buf);
        free(ids);
        return 1;
    }

    index--;

    for (uint32_t i = index; i < count - 1; i++) {
        ids[i] = ids[i + 1];
        name_offsets[i] = name_offsets[i + 1];
        memcpy(buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)), &ids[i], sizeof(uint32_t));
        memcpy(buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t),
                &name_offsets[i], sizeof(uint32_t));
    }

    // move names
    int name_offset = sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t));
    memmove(buf + name_offset - (sizeof(uint32_t) + sizeof(uint32_t)), buf + name_offset, size - name_offset);

    // TODO: remove the name of the removed element

    // update the number of elements
    count--;
    memcpy(buf, &count, sizeof(uint32_t));

    // write the directory
    if (syscall_fs_write(NULL, dir_sid, buf, 0,
        size - (sizeof(uint32_t) + sizeof(uint32_t)))
    ) {
        RAISE_ERROR("remove_element_from_dir: failed to write to d%ds%d\n", SID_DISK(dir_sid), SID_SECTOR(dir_sid));
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

uint32_t fu_dir_create(int device_id, char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    head_sid = fu_path_to_sid(SID_ROOT, path);
    if (!IS_SID_NULL(head_sid)) {
        RAISE_ERROR("dir_create: '%s' already exists\n", path);
        return SID_NULL;
    }

    fu_sep_path(path, &parent, &name);
    if (parent[0]) {
        parent_sid = fu_path_to_sid(SID_ROOT, parent);
        if (IS_SID_NULL(parent_sid)) {
            RAISE_ERROR("dir_create: failed to find parent directory of '%s'\n", path);
            free(parent);
            free(name);
            return SID_NULL;
        }
        if (!fu_is_dir(parent_sid)) {
            RAISE_ERROR("dir_create: parent of '%s' is not a directory\n", path);
            free(parent);
            free(name);
            return SID_NULL;
        }
    } else {
        parent_sid = SID_ROOT;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    strcpy(meta, "D-");
    strncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        RAISE_ERROR("dir_create: failed to create directory '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // create a link in parent directory
    if (parent[0]) {
        if (fu_add_to_dir(parent_sid, head_sid, name)) {
            RAISE_ERROR("dir_create: failed to add directory '%s' to parent\n", path);
            free(parent);
            free(name);
            return SID_NULL;
        }
    }

    syscall_fs_set_size(NULL, head_sid, sizeof(uint32_t));
    syscall_fs_write(NULL, head_sid, "\0\0\0\0", 0, 4);

    // create '.' and '..'
    if (fu_add_to_dir(head_sid, parent_sid, "..")) {
        RAISE_ERROR("dir_create: failed to add '..' to directory '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    if (fu_add_to_dir(head_sid, head_sid, ".")) {
        RAISE_ERROR("dir_create: failed to add '.' to directory '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
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

int fu_is_file(uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid)) return 0;

    char *name = syscall_fs_meta(NULL, dir_sid, NULL);
    if (name == NULL) return 0;
    if (name[0] == 'F') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

int fu_file_set_size(uint32_t file_sid, uint32_t size) {
    if (!fu_is_file(file_sid)) {
        RAISE_ERROR("file_set_size: d%ds%d is not a file\n", SID_DISK(file_sid), SID_SECTOR(file_sid));
        return 1;
    }

    return syscall_fs_set_size(NULL, file_sid, size);
}

uint32_t fu_file_get_size(uint32_t file_sid) {
    if (!fu_is_file(file_sid)) {
        RAISE_ERROR("file_get_size: d%ds%d is not a file\n", SID_DISK(file_sid), SID_SECTOR(file_sid));
        return UINT32_MAX;
    }

    return syscall_fs_get_size(NULL, file_sid);
}

uint32_t fu_file_create(int device_id, const char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    head_sid = fu_path_to_sid(SID_ROOT, path);
    if (!IS_SID_NULL(head_sid)) {
        RAISE_ERROR("file_create: '%s' already exists\n", path);
        return SID_NULL;
    }

    fu_sep_path(path, &parent, &name);
    if (!parent[0]) {
        RAISE_ERROR("file_create: parent of '%s' is unreachable\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    if (IS_SID_NULL(parent_sid)) {
        RAISE_ERROR("file_create: failed to find parent directory of '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }
    if (!fu_is_dir(parent_sid)) {
        RAISE_ERROR("file_create: parent of '%s' is not a directory\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    strcpy(meta, "F-");
    strncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        RAISE_ERROR("file_create: failed to initialize container '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // create a link in parent directory
    if (fu_add_to_dir(parent_sid, head_sid, name)) {
        RAISE_ERROR("file_create: failed to add file '%s' to parent\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    free(parent);
    free(name);

    return head_sid;
}

int fu_file_read(uint32_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || syscall_fs_read(NULL, file_sid, buf, offset, size));
}

int fu_file_write(uint32_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || syscall_fs_write(NULL, file_sid, buf, offset, size));
}

/**************************************************
 *                                               *
 *      Function Calls Through File System       *
 *                                               *
**************************************************/

// call a function when reading a file

int fu_is_fctf(uint32_t file_sid) {
    if (IS_SID_NULL(file_sid)) return 0;

    char *name = syscall_fs_meta(NULL, file_sid, NULL);
    if (name == NULL) return 0;
    if (name[0] == 'C') {
        free(name);
        return 1;
    }
    free(name);
    return 0;
}

uint32_t fu_fctf_create(int device_id, char *path, int (*fct)(int, void *, uint32_t, uint8_t)) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    head_sid = fu_path_to_sid(SID_ROOT, path);
    if (!IS_SID_NULL(head_sid)) {
        RAISE_ERROR("fctf_create: '%s' already exists\n", path);
        return SID_NULL;
    }

    fu_sep_path(path, &parent, &name);
    if (!parent[0]) {
        RAISE_ERROR("fctf_create: parent of '%s' is unreachable\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    if (IS_SID_NULL(parent_sid)) {
        RAISE_ERROR("fctf_create: failed to find parent directory of '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }
    if (!fu_is_dir(parent_sid)) {
        RAISE_ERROR("fctf_create: parent of '%s' is not a directory\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    strcpy(meta, "C-");
    strncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        RAISE_ERROR("fctf_create: failed to initialize container '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // create a link in parent directory
    if (fu_add_to_dir(parent_sid, head_sid, name)) {
        RAISE_ERROR("fctf_create: failed to add file '%s' to parent\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    // write the function pointer
    syscall_fs_set_size(NULL, head_sid, sizeof(void *));
    if (syscall_fs_write(NULL, head_sid, (void *) &fct, 0, sizeof(void *))) {
        RAISE_ERROR("fctf_create: failed to write function pointer to '%s'\n", path);
        free(parent);
        free(name);
        return SID_NULL;
    }

    free(parent);
    free(name);

    return head_sid;
}

uint32_t fu_fctf_get_addr(uint32_t file_sid) {
    uint32_t addr;

    // read container
    if (syscall_fs_read(NULL, file_sid, &addr, 0, sizeof(void *))) {
        RAISE_ERROR("fctf_get_addr: failed to read function pointer from d%ds%d\n",
                SID_DISK(file_sid), SID_SECTOR(file_sid));
        return 1;
    }

    return addr;
}

/**************************************************
 *                                               *
 *            Path to SID conversion             *
 *                                               *
**************************************************/
static uint32_t fu_rec_path_to_sid(filesys_t *filesys, uint32_t parent, const char *path) {
    uint32_t ret = SID_NULL;

    // generate the path part to search for
    char *name = malloc(strlen(path) + 1);
    strcpy(name, path);

    uint32_t i = 0;
    for (; name[i]; i++) {
        if (name[i] != '/')
            continue;
        name[i] = '\0';
        break;
    }

    for (; path[i] == '/'; i++);

    // get the directory content
    uint32_t *sids;
    char **names;
    int count;

    count = fu_get_dir_content(parent, &sids, &names);

    if (count == -1) {
        RAISE_ERROR("rec_path_to_sid: failed to get directory content of d%ds%d\n",
                SID_DISK(parent),
                SID_SECTOR(parent)
        );
        free(name);
        return SID_NULL;
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
    for (int j = 0; j < count; j++)
        free(names[j]);
    free(names);
    free(sids);
    free(name);

    return ret;
}

uint32_t fu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (strcmp("/", path) == 0)
        return from;

    int len = strlen(path) - 1;
    char *tmp;

    if (path[len] == '/') {
        tmp = strdup(path);
        tmp[len] = '\0';
    } else {
        tmp = (char *) path;
    }

    ret = fu_rec_path_to_sid(NULL, from, tmp + (tmp[0] == '/'));

    if (tmp != path)
        free(tmp);

    return ret;
}

void fu_simplify_path(char *path) {
    // some path look like this: /a/b/../c/./d/./e/../f
    // this function simplifies them to: /a/c/d/f

    if (path[0] != '/') {
        RAISE_ERROR("simplify_path: cannot simplify relative path '%s'\n", path);
        return;
    }

    char *tmp = malloc(strlen(path) + 2);
    strcpy(tmp, path);
    strcat(tmp, "/");

    int i;
    for (i = 0; tmp[i]; i++) {
        if (tmp[i] == '/' && tmp[i + 1] == '/') {
            memmove(tmp + i, tmp + i + 1, strlen(tmp + i));
            i--;
            continue;
        }
        if (tmp[i] == '/' && tmp[i + 1] == '.' && tmp[i + 2] == '/') {
            memmove(tmp + i, tmp + i + 2, strlen(tmp + i));
            i--;
            continue;
        }
        if (tmp[i] == '/' && tmp[i + 1] == '.' && tmp[i + 2] == '.' && tmp[i + 3] == '/') {
            if (i == 0) {
                memmove(tmp, tmp + 3, strlen(tmp + 2));
                i = -1;
                continue;
            }
            int j = i - 1;
            while (j >= 0 && tmp[j] != '/') j--;
            if (j >= 0) {
                memmove(tmp + j, tmp + i + 3, strlen(tmp + i));
                i = j - 1;
            }
            continue;
        }
    }
    if (tmp[i - 1] == '/' && i > 1) {
        tmp[i - 1] = '\0';
    }
    strcpy(path, tmp);
    free(tmp);
}

/**************************************************
 *                                               *
 *             File System Get Info              *
 *                                               *
**************************************************/

uint32_t *fu_get_vdisk_info(void) {
    // array:
    // [0] = vdisk_count
    // [3n + 1] = vdisk[n] mount point
    // [3n + 2] = vdisk[n] used_count
    // [3n + 3] = vdisk[n] size
    // ...

    filesys_t *filesys = syscall_fs_get_default();

    uint32_t *ret = malloc(sizeof(uint32_t) * (filesys->vdisk_count * 3 + 1));
    ret[0] = filesys->vdisk_count;
    int ret_i = 1;
    for (uint32_t i = 0; i < FS_MAX_DISKS; i++) {
        if (filesys->vdisk[i] == NULL) continue;
        ret[ret_i] = i + 1;
        ret[ret_i + 1] = filesys->vdisk[i]->used_count;
        ret[ret_i + 2] = filesys->vdisk[i]->size;
        ret_i += 3;
    }

    return ret;
}
