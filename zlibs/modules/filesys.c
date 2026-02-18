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

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

#include <errno.h>

#define FILESYS_DEBUG 1 // 0: no debug, 1: debug

/********************************************
 *                                         *
 *          Definitions and macros         *
 *                                         *
********************************************/

#if FILESYS_DEBUG
#define ERROR_EXISTS  (sys_warning("filesys mod: line %d: file already exists\n", __LINE__), -EEXIST)
#define ERROR_NOT_DIR (sys_warning("filesys mod: line %d: not a directory\n", __LINE__), -ENOTDIR)
#define ERROR_INV_ARG (sys_warning("filesys mod: line %d: invalid argument\n", __LINE__), -EINVAL)
#define ERROR_INTERN  (sys_warning("filesys mod: line %d: internal error\n", __LINE__), -EIO)
#define ERROR_NOT_FND (sys_warning("filesys mod: line %d: not found\n", __LINE__), -ENOENT)
#else
#define ERROR_EXISTS  -EEXIST
#define ERROR_NOT_DIR -ENOTDIR
#define ERROR_INV_ARG -EINVAL
#define ERROR_INTERN  -EIO
#define ERROR_NOT_FND -ENOENT
#endif

uint32_t fu_path_to_sid(uint32_t from, const char *path);

/********************************************
 *                                         *
 *          Auxiliary functions            *
 *                                         *
********************************************/

static void fu_sep_path(const char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = str_len(fullpath);

    if (parent != NULL) {
        *parent = calloc(len + 2);
    }

    if (cnt != NULL) {
        *cnt = calloc(len + 2);
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
            str_cpy(*parent, "/");
        } else {
            str_ncpy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        str_cpy(*cnt, fullpath + i + 1);
    }
}

/********************************************
 *                                         *
 *      Standard directory operations      *
 *                                         *
********************************************/

int fu_is_dir(uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid))
        return 0;

    char *name = fs_cnt_meta(dir_sid, NULL);

    if (name == NULL)
        return 0;

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

int fu_dir_get_size(uint32_t dir_sid) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return ERROR_INTERN;

    // read the directory
    uint32_t count;

    if (fs_cnt_read(dir_sid, &count, 0, sizeof(uint32_t)))
        return ERROR_INTERN;

    return count;
}

int fu_dir_get_elm(uint8_t *buf, uint32_t bsize, uint32_t index, uint32_t *sid) {
    // positive return: name offset
    // zero return: end of directory
    // negative return: error code (-errno)

    if (sid != NULL)
        *sid = SID_NULL;

    if (bsize < sizeof(uint32_t))
        return ERROR_INTERN;

    uint32_t count;
    mem_copy(&count, buf, sizeof(uint32_t));

    if (count <= index)
        return 0;

    uint32_t name_offset;
    uint32_t offset;

    offset = sizeof(uint32_t) + index * (sizeof(uint32_t) * 2);
    if (bsize < offset + sizeof(uint32_t) * 2)
        return ERROR_INTERN;

    if (sid != NULL)
        mem_copy(sid, buf + offset, sizeof(uint32_t));

    mem_copy(&name_offset, buf + offset + sizeof(uint32_t), sizeof(uint32_t));
    offset = sizeof(uint32_t) + count * (sizeof(uint32_t) * 2) + name_offset;

    if (bsize < offset)
        return ERROR_INTERN;

    return offset;
}

int fu_dir_get_content(uint32_t dir_sid, uint32_t **ids, char ***names) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    uint32_t sid;
    int offset;

    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return ERROR_INTERN;
    }

    uint8_t *buf = malloc(size);
    if (fs_cnt_read(dir_sid, buf, 0, size)) {
        free(buf);
        return ERROR_INTERN;
    }

    // get the number of elements
    uint32_t count;
    count = *(uint32_t *) buf;

    if (count == 0 || ids == NULL || names == NULL) {
        free(buf);
        return count;
    }

    // get the elements
    *ids = malloc(sizeof(uint32_t) * count);
    *names = malloc(sizeof(char *) * count);

    for (uint32_t i = 0; i < count; i++) {
        offset = fu_dir_get_elm(buf, size, i, &sid);
        if (offset <= 0) {
            free(*ids);
            free(*names);
            free(buf);
            return ERROR_INTERN;
        }

        (*ids)[i] = sid;
        (*names)[i] = malloc(str_len((char *) buf + offset) + 1);
        str_cpy((*names)[i], (char *) buf + offset);
    }

    free(buf);

    return count;
}

int fu_add_to_dir(uint32_t dir_sid, uint32_t element_sid, char *name) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    // check if the name is valid
    for (int i = 0; name[i]; i++) {
        if (name[i] == '/')
            return ERROR_INV_ARG;
    }

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return ERROR_INTERN;

    if (fs_cnt_set_size(dir_sid, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1))
        return ERROR_INTERN;

    // read the directory
    uint8_t *buf = malloc(size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1);
    if (fs_cnt_read(dir_sid, buf, 0, size))
        return ERROR_INTERN;

    uint32_t count = 0;
    mem_copy(&count, buf, sizeof(uint32_t));

    // move names
    if (count > 0) {
        for (uint32_t i = size - 1; i >= sizeof(uint32_t) + count * (sizeof(uint32_t) * 2); i--) {
            buf[i + sizeof(uint32_t) * 2] = buf[i];
        }
    }

    uint32_t offset = sizeof(uint32_t) + count * (sizeof(uint32_t) * 2);

    // insert the new element
    mem_copy(buf + offset, &element_sid, sizeof(uint32_t));

    uint32_t name_offset = size - sizeof(uint32_t) - count * (sizeof(uint32_t) * 2);
    mem_copy(buf + offset + sizeof(uint32_t), &name_offset, sizeof(uint32_t));

    // update the number of elements
    count++;
    mem_copy(buf, &count, sizeof(uint32_t));

    str_cpy((char *) buf + sizeof(uint32_t) + count * sizeof(uint32_t) * 2 + name_offset, name);

    // write the directory
    if (fs_cnt_write(dir_sid, buf, 0, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1))
        return ERROR_INTERN;

    free(buf);
    return 0;
}

int fu_remove_from_dir(uint32_t dir_sid, uint32_t element_sid) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX)
        return ERROR_INTERN;

    // read the directory
    uint8_t *buf = malloc(size);
    if (fs_cnt_read(dir_sid, buf, 0, size))
        return ERROR_INTERN;

    uint32_t count = *(uint32_t *) buf;

    if (count == 0) {
        free(buf);
        return ERROR_NOT_FND;
    }

    // get the elements
    uint32_t *ids = malloc(sizeof(uint32_t) * count);
    uint32_t *name_offsets = malloc(sizeof(uint32_t) * count);

    for (uint32_t i = 0; i < count; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        mem_copy(&ids[i], addr, sizeof(uint32_t));
        mem_copy(&name_offsets[i], addr + sizeof(uint32_t), sizeof(uint32_t));
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
        free(name_offsets);
        free(buf);
        free(ids);
        return ERROR_NOT_FND;
    }

    index--;

    for (uint32_t i = index; i < count - 1; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        ids[i] = ids[i + 1];
        name_offsets[i] = name_offsets[i + 1];

        mem_copy(addr, &ids[i], sizeof(uint32_t));
        mem_copy(addr + sizeof(uint32_t), &name_offsets[i], sizeof(uint32_t));
    }

    free(name_offsets);
    free(ids);

    // move names
    int name_offset = sizeof(uint32_t) + count * sizeof(uint32_t) * 2;
    mem_move(buf + name_offset - sizeof(uint32_t) * 2, buf + name_offset, size - name_offset);

    // TODO: remove the name of the removed element

    // update the number of elements
    count--;
    mem_copy(buf, &count, sizeof(uint32_t));

    // write the directory
    if (fs_cnt_write(dir_sid, buf, 0, size - sizeof(uint32_t) * 2)) {
        free(buf);
        return ERROR_INTERN;
    }

    free(buf);
    return 0;
}

uint32_t fu_dir_create(int device_id, const char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    head_sid = fu_path_to_sid(SID_ROOT, path);
    if (!IS_SID_NULL(head_sid)) {
        return (ERROR_EXISTS, SID_NULL);
    }

    fu_sep_path(path, &parent, &name);
    if (parent[0]) {
        parent_sid = fu_path_to_sid(SID_ROOT, parent);
        if (!fu_is_dir(parent_sid)) {
            free(parent);
            free(name);
            return (ERROR_NOT_DIR, SID_NULL);
        }
    } else {
        parent_sid = SID_ROOT;
    }

    int namelen = str_len(name);

    while (namelen > 0 && name[namelen - 1] == '/')
        namelen--;

    if (namelen == 0) {
        free(parent);
        free(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    name[namelen] = '\0';

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "D-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = fs_cnt_init((device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        free(parent);
        free(name);
        return (ERROR_INTERN, SID_NULL);
    }

    // create a link in parent directory
    if (parent[0] && fu_add_to_dir(parent_sid, head_sid, name)) {
        free(parent);
        free(name);
        return (ERROR_INTERN, SID_NULL);
    }

    free(parent);
    free(name);

    fs_cnt_set_size(head_sid, sizeof(uint32_t));
    fs_cnt_write(head_sid, "\0\0\0\0", 0, 4);

    // create '.' and '..'
    if (fu_add_to_dir(head_sid, parent_sid, "..") || fu_add_to_dir(head_sid, head_sid, "."))
        return (ERROR_INTERN, SID_NULL);

    return head_sid;
}

/********************************************
 *                                         *
 *        Standard file operations         *
 *                                         *
********************************************/

int fu_is_file(uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid))
        return 0;

    char *name = fs_cnt_meta(dir_sid, NULL);

    if (name == NULL)
        return 0;

    if (name[0] == 'F') {
        free(name);
        return 1;
    }

    free(name);
    return 0;
}

int fu_file_set_size(uint32_t file_sid, uint32_t size) {
    if (!fu_is_file(file_sid))
        return ERROR_INV_ARG;

    if (fs_cnt_set_size(file_sid, size))
        return ERROR_INTERN;

    return 0;
}

uint32_t fu_file_get_size(uint32_t file_sid) {
    if (!fu_is_file(file_sid))
        return (ERROR_INV_ARG, UINT32_MAX);

    return fs_cnt_get_size(file_sid);
}

uint32_t fu_file_create(int device_id, const char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    head_sid = fu_path_to_sid(SID_ROOT, path);
    if (!IS_SID_NULL(head_sid))
        return (ERROR_EXISTS, SID_NULL);

    fu_sep_path(path, &parent, &name);
    if (!parent[0]) {
        free(parent);
        free(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);

    if (!fu_is_dir(parent_sid)) {
        free(name);
        return (ERROR_NOT_DIR, SID_NULL);
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "F-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = fs_cnt_init((device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    // create a link in parent directory
    if (IS_SID_NULL(head_sid) || fu_add_to_dir(parent_sid, head_sid, name)) {
        free(name);
        return (ERROR_INTERN, SID_NULL);
    }

    free(name);
    return head_sid;
}

int fu_file_read(uint32_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || fs_cnt_read(file_sid, buf, offset, size));
}

int fu_file_write(uint32_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!fu_is_file(file_sid) || fs_cnt_write(file_sid, buf, offset, size));
}

/**************************************************
 *                                               *
 *      Function Calls Through File System       *
 *                                               *
**************************************************/

// call a function when reading a file

int fu_is_afft(uint32_t file_sid) {
    if (IS_SID_NULL(file_sid))
        return 0;

    char *name = fs_cnt_meta(file_sid, NULL);

    if (name == NULL)
        return 0;

    if (name[0] == 'A') {
        free(name);
        return 1;
    }

    free(name);
    return 0;
}

uint32_t fu_afft_create(int device_id, const char *path, uint32_t id) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    if (!IS_SID_NULL(fu_path_to_sid(SID_ROOT, path)))
        return (ERROR_EXISTS, SID_NULL);

    fu_sep_path(path, &parent, &name);

    if (!parent[0]) {
        free(parent);
        free(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    free(parent);

    if (!fu_is_dir(parent_sid)) {
        free(name);
        return (ERROR_NOT_DIR, SID_NULL);
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "A-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = fs_cnt_init((device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    // create a link in parent directory
    if (IS_SID_NULL(head_sid) || fu_add_to_dir(parent_sid, head_sid, name)) {
        free(name);
        return (ERROR_INTERN, SID_NULL);
    }

    free(name);

    // write the function pointer
    fs_cnt_set_size(head_sid, sizeof(uint32_t));
    if (fs_cnt_write(head_sid, (void *) &id, 0, sizeof(uint32_t)))
        return SID_NULL;

    return head_sid;
}

int fu_afft_get_id(uint32_t file_sid) {
    int id;

    if (!fu_is_afft(file_sid) || fs_cnt_get_size(file_sid) != sizeof(uint32_t))
        return (ERROR_INTERN, -1);

    // read container
    if (fs_cnt_read(file_sid, &id, 0, sizeof(uint32_t)))
        return (ERROR_INTERN, -1);

    return id;
}

/**************************************************
 *                                               *
 *            Path to SID conversion             *
 *                                               *
**************************************************/

static int path_to_sid_cmp(const char *path, const char *name, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        if (name[i] == '\0' || path[i] != name[i])
            return 1;
    }

    return name[len] != '\0';
}

static uint32_t rec_path_to_sid(uint32_t parent, const char *path) {
    // generate the path part to search for
    uint32_t path_len = 0;

    while (*path == '/')
        path++;

    while (path[path_len] && path[path_len] != '/')
        path_len++;

    if (path_len == 0)
        return parent;

    // get the directory content
    uint32_t sid, size;
    uint8_t *buf;
    int offset;

    size = fs_cnt_get_size(parent);

    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return (ERROR_INTERN, SID_NULL);

    buf = malloc(size);

    if (fs_cnt_read(parent, buf, 0, size)) {
        free(buf);
        return (ERROR_INTERN, SID_NULL);
    }

    // search for the path part
    for (int j = 0; (offset = fu_dir_get_elm(buf, size, j, &sid)) > 0; j++) {
        if (str_cmp(path, (char *) buf + offset) == 0) {
            free(buf);
            return sid;
        }

        if (path_to_sid_cmp(path, (char *) buf + offset, path_len) == 0 && fu_is_dir(sid)) {
            free(buf);
            return rec_path_to_sid(sid, path + path_len);
        }
    }

    free(buf);

    return SID_NULL;
}

uint32_t fu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (path[0] == '/' && path[1] == '\0')
        return from;

    ret = rec_path_to_sid(from, path);

    return ret;
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

    filesys_t *filesys = fs_get_filesys();

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

void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    fu_is_dir,
    fu_dir_get_size,
    fu_dir_get_elm,
    fu_dir_get_content,
    fu_add_to_dir,
    fu_remove_from_dir,
    fu_dir_create,
    fu_is_file,
    fu_file_set_size,
    fu_file_get_size,
    fu_file_create,
    fu_file_read,
    fu_file_write,
    fu_is_afft,
    fu_afft_create,
    fu_afft_get_id,
    fu_path_to_sid,
    fu_get_vdisk_info
};
