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

#include <errno.h>

#define FILESYS_LIB_C
#include <profan/filesys.h>

#define FILESYS_DEBUG 1 // 0: no debug, 1: debug

/********************************************
 *                                         *
 *          Definitions and macros         *
 *                                         *
********************************************/

#if FILESYS_DEBUG
#define ERROR_EXISTS  (fd_printf(2, "filesys: line %d: file already exists\n", __LINE__), -EEXIST)
#define ERROR_NOT_DIR (fd_printf(2, "filesys: line %d: not a directory\n", __LINE__), -ENOTDIR)
#define ERROR_INV_ARG (fd_printf(2, "filesys: line %d: invalid argument\n", __LINE__), -EINVAL)
#define ERROR_INTERN  (fd_printf(2, "filesys: line %d: internal error\n", __LINE__), -EIO)
#define ERROR_NOT_FND (fd_printf(2, "filesys: line %d: not found\n", __LINE__), -ENOENT)
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

// init function
int main(void) {
    return 0;
}

static void fu_sep_path(const char *fullpath, char **parent, char **cnt) {
    int i, len;

    len = str_len(fullpath);

    if (parent != NULL) {
        *parent = kcalloc(1, len + 2);
    }

    if (cnt != NULL) {
        *cnt = kcalloc(1, len + 2);
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

    char *name = syscall_fs_meta(NULL, dir_sid, NULL);

    if (name == NULL)
        return 0;

    if (name[0] == 'D') {
        kfree(name);
        return 1;
    }

    kfree(name);
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

int fu_get_dir_size(uint32_t dir_sid) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    // read the directory and get size
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return ERROR_INTERN;

    // read the directory
    uint32_t count;

    if (syscall_fs_read(NULL, dir_sid, &count, 0, sizeof(uint32_t)))
        return ERROR_INTERN;

    return count;
}

int fu_get_dir_elm(uint8_t *buf, uint32_t bsize, uint32_t index, uint32_t *sid) {
    // positive return: name offset
    // zero return: end of directory
    // negative return: error code (-errno)

    if (sid != NULL)
        *sid = SID_NULL;

    if (bsize < sizeof(uint32_t))
        return ERROR_INTERN;

    uint32_t count;
    mem_cpy(&count, buf, sizeof(uint32_t));

    if (count <= index)
        return 0;

    uint32_t name_offset;
    uint32_t offset;

    offset = sizeof(uint32_t) + index * (sizeof(uint32_t) * 2);
    if (bsize < offset + sizeof(uint32_t) * 2)
        return ERROR_INTERN;

    if (sid != NULL)
        mem_cpy(sid, buf + offset, sizeof(uint32_t));

    mem_cpy(&name_offset, buf + offset + sizeof(uint32_t), sizeof(uint32_t));
    offset = sizeof(uint32_t) + count * (sizeof(uint32_t) * 2) + name_offset;

    if (bsize < offset)
        return ERROR_INTERN;

    return offset;
}

int fu_get_dir_content(uint32_t dir_sid, uint32_t **ids, char ***names) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    uint32_t sid;
    int offset;

    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return ERROR_INTERN;
    }

    uint8_t *buf = kmalloc(size);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size)) {
        kfree(buf);
        return ERROR_INTERN;
    }

    // get the number of elements
    uint32_t count;
    count = *(uint32_t *) buf;

    if (count == 0 || ids == NULL || names == NULL) {
        kfree(buf);
        return count;
    }

    // get the elements
    *ids = kmalloc(sizeof(uint32_t) * count);
    *names = kmalloc(sizeof(char *) * count);

    for (uint32_t i = 0; i < count; i++) {
        offset = fu_get_dir_elm(buf, size, i, &sid);
        if (offset <= 0) {
            kfree(*ids);
            kfree(*names);
            kfree(buf);
            return ERROR_INTERN;
        }

        (*ids)[i] = sid;
        (*names)[i] = kmalloc(str_len((char *) buf + offset) + 1);
        str_cpy((*names)[i], (char *) buf + offset);
    }

    kfree(buf);

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
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return ERROR_INTERN;

    if (syscall_fs_set_size(NULL, dir_sid, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1))
        return ERROR_INTERN;

    // read the directory
    uint8_t *buf = kmalloc(size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size))
        return ERROR_INTERN;

    uint32_t count = 0;
    mem_cpy(&count, buf, sizeof(uint32_t));

    // move names
    if (count > 0) {
        for (uint32_t i = size - 1; i >= sizeof(uint32_t) + count * (sizeof(uint32_t) * 2); i--) {
            buf[i + sizeof(uint32_t) * 2] = buf[i];
        }
    }

    uint32_t offset = sizeof(uint32_t) + count * (sizeof(uint32_t) * 2);

    // insert the new element
    mem_cpy(buf + offset, &element_sid, sizeof(uint32_t));

    uint32_t name_offset = size - sizeof(uint32_t) - count * (sizeof(uint32_t) * 2);
    mem_cpy(buf + offset + sizeof(uint32_t), &name_offset, sizeof(uint32_t));

    // update the number of elements
    count++;
    mem_cpy(buf, &count, sizeof(uint32_t));

    str_cpy((char *) buf + sizeof(uint32_t) + count * sizeof(uint32_t) * 2 + name_offset, name);

    // write the directory
    if (syscall_fs_write(NULL, dir_sid, buf, 0, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1))
        return ERROR_INTERN;

    kfree(buf);
    return 0;
}

int fu_remove_from_dir(uint32_t dir_sid, uint32_t element_sid) {
    if (!fu_is_dir(dir_sid))
        return ERROR_NOT_DIR;

    // read the directory and get size
    uint32_t size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX)
        return ERROR_INTERN;

    // read the directory
    uint8_t *buf = kmalloc(size);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size))
        return ERROR_INTERN;

    uint32_t count = *(uint32_t *) buf;

    if (count == 0) {
        kfree(buf);
        return ERROR_NOT_FND;
    }

    // get the elements
    uint32_t *ids = kmalloc(sizeof(uint32_t) * count);
    uint32_t *name_offsets = kmalloc(sizeof(uint32_t) * count);

    for (uint32_t i = 0; i < count; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        mem_cpy(&ids[i], addr, sizeof(uint32_t));
        mem_cpy(&name_offsets[i], addr + sizeof(uint32_t), sizeof(uint32_t));
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
        kfree(name_offsets);
        kfree(buf);
        kfree(ids);
        return ERROR_NOT_FND;
    }

    index--;

    for (uint32_t i = index; i < count - 1; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        ids[i] = ids[i + 1];
        name_offsets[i] = name_offsets[i + 1];

        mem_cpy(addr, &ids[i], sizeof(uint32_t));
        mem_cpy(addr + sizeof(uint32_t), &name_offsets[i], sizeof(uint32_t));
    }

    kfree(name_offsets);
    kfree(ids);

    // move names
    int name_offset = sizeof(uint32_t) + count * sizeof(uint32_t) * 2;
    mem_move(buf + name_offset - sizeof(uint32_t) * 2, buf + name_offset, size - name_offset);

    // TODO: remove the name of the removed element

    // update the number of elements
    count--;
    mem_cpy(buf, &count, sizeof(uint32_t));

    // write the directory
    if (syscall_fs_write(NULL, dir_sid, buf, 0, size - sizeof(uint32_t) * 2)) {
        kfree(buf);
        return ERROR_INTERN;
    }

    kfree(buf);
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
            kfree(parent);
            kfree(name);
            return (ERROR_NOT_DIR, SID_NULL);
        }
    } else {
        parent_sid = SID_ROOT;
    }

    int namelen = str_len(name);

    while (namelen > 0 && name[namelen - 1] == '/')
        namelen--;

    if (namelen == 0) {
        kfree(parent);
        kfree(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    name[namelen] = '\0';

    // generate the meta
    char *meta = kmalloc(META_MAXLEN);
    str_cpy(meta, "D-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    kfree(meta);

    if (IS_SID_NULL(head_sid)) {
        kfree(parent);
        kfree(name);
        return (ERROR_INTERN, SID_NULL);
    }

    // create a link in parent directory
    if (parent[0] && fu_add_to_dir(parent_sid, head_sid, name)) {
        kfree(parent);
        kfree(name);
        return (ERROR_INTERN, SID_NULL);
    }

    kfree(parent);
    kfree(name);

    syscall_fs_set_size(NULL, head_sid, sizeof(uint32_t));
    syscall_fs_write(NULL, head_sid, "\0\0\0\0", 0, 4);

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

    char *name = syscall_fs_meta(NULL, dir_sid, NULL);

    if (name == NULL)
        return 0;

    if (name[0] == 'F') {
        kfree(name);
        return 1;
    }

    kfree(name);
    return 0;
}

int fu_file_set_size(uint32_t file_sid, uint32_t size) {
    if (!fu_is_file(file_sid))
        return ERROR_INV_ARG;

    if (syscall_fs_set_size(NULL, file_sid, size))
        return ERROR_INTERN;

    return 0;
}

uint32_t fu_file_get_size(uint32_t file_sid) {
    if (!fu_is_file(file_sid))
        return (ERROR_INV_ARG, UINT32_MAX);

    return syscall_fs_get_size(NULL, file_sid);
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
        kfree(parent);
        kfree(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    kfree(parent);

    if (!fu_is_dir(parent_sid)) {
        kfree(name);
        return (ERROR_NOT_DIR, SID_NULL);
    }

    // generate the meta
    char *meta = kmalloc(META_MAXLEN);
    str_cpy(meta, "F-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    kfree(meta);

    // create a link in parent directory
    if (IS_SID_NULL(head_sid) || fu_add_to_dir(parent_sid, head_sid, name)) {
        kfree(name);
        return (ERROR_INTERN, SID_NULL);
    }

    kfree(name);
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
    if (IS_SID_NULL(file_sid))
        return 0;

    char *name = syscall_fs_meta(NULL, file_sid, NULL);

    if (name == NULL)
        return 0;

    if (name[0] == 'C') {
        kfree(name);
        return 1;
    }

    kfree(name);
    return 0;
}

uint32_t fu_fctf_create(int device_id, const char *path, int (*fct)(int, void *, uint32_t, uint8_t)) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    // check if the the path already exists
    if (!IS_SID_NULL(fu_path_to_sid(SID_ROOT, path)))
        return (ERROR_EXISTS, SID_NULL);

    fu_sep_path(path, &parent, &name);

    if (!parent[0]) {
        kfree(parent);
        kfree(name);
        return (ERROR_INV_ARG, SID_NULL);
    }

    parent_sid = fu_path_to_sid(SID_ROOT, parent);
    kfree(parent);

    if (!fu_is_dir(parent_sid)) {
        kfree(name);
        return (ERROR_NOT_DIR, SID_NULL);
    }

    // generate the meta
    char *meta = kmalloc(META_MAXLEN);
    str_cpy(meta, "C-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);
    meta[META_MAXLEN - 1] = '\0';

    head_sid = syscall_fs_init(NULL, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    kfree(meta);

    // create a link in parent directory
    if (IS_SID_NULL(head_sid) || fu_add_to_dir(parent_sid, head_sid, name)) {
        kfree(name);
        return (ERROR_INTERN, SID_NULL);
    }

    kfree(name);

    // write the function pointer
    syscall_fs_set_size(NULL, head_sid, sizeof(void *));
    if (syscall_fs_write(NULL, head_sid, (void *) &fct, 0, sizeof(void *)))
        return SID_NULL;

    return head_sid;
}

void *fu_fctf_get_addr(uint32_t file_sid) {
    void *addr;

    if (!fu_is_fctf(file_sid) || syscall_fs_get_size(NULL, file_sid) < sizeof(void *))
        return (ERROR_INTERN, NULL);

    // read container
    if (syscall_fs_read(NULL, file_sid, &addr, 0, sizeof(void *)))
        return (ERROR_INTERN, NULL);

    return addr;
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

    // get the directory content
    uint32_t sid;
    int offset;

    uint32_t size = syscall_fs_get_size(NULL, parent);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return (ERROR_INTERN, SID_NULL);
    }

    uint8_t *buf = kmalloc(size);
    if (syscall_fs_read(NULL, parent, buf, 0, size)) {
        kfree(buf);
        return (ERROR_INTERN, SID_NULL);
    }

    // search for the path part
    for (int j = 0; (offset = fu_get_dir_elm(buf, size, j, &sid)) > 0; j++) {
        if (str_cmp(path, (char *) buf + offset) == 0) {
            kfree(buf);
            return sid;
        }

        if (path_to_sid_cmp(path, (char *) buf + offset, path_len) == 0 && fu_is_dir(sid)) {
            kfree(buf);
            return rec_path_to_sid(sid, path + path_len);
        }
    }

    kfree(buf);
    return SID_NULL;
}

uint32_t fu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (str_cmp("/", path) == 0)
        return from;

    int len = str_len(path) - 1;
    char *tmp;

    if (path[len] == '/') {
        tmp = str_dup(path);
        tmp[len] = '\0';
    } else {
        tmp = (char *) path;
    }

    ret = rec_path_to_sid(from, tmp + (tmp[0] == '/'));

    if (tmp != path)
        kfree(tmp);

    return ret;
}

int fu_simplify_path(char *path) {
    // some path look like this: /a/b/../c/./d/./e/../f
    // this function simplifies them to: /a/c/d/f

    if (path[0] != '/') {
        return ERROR_INV_ARG;
    }

    char *tmp = kmalloc(str_len(path) + 2);
    str_cpy(tmp, path);
    str_cat(tmp, "/");

    int i;
    for (i = 0; tmp[i]; i++) {
        if (tmp[i] == '/' && tmp[i + 1] == '/') {
            mem_move(tmp + i, tmp + i + 1, str_len(tmp + i));
            i--;
            continue;
        }
        if (tmp[i] == '/' && tmp[i + 1] == '.' && tmp[i + 2] == '/') {
            mem_move(tmp + i, tmp + i + 2, str_len(tmp + i));
            i--;
            continue;
        }
        if (tmp[i] == '/' && tmp[i + 1] == '.' && tmp[i + 2] == '.' && tmp[i + 3] == '/') {
            if (i == 0) {
                mem_move(tmp, tmp + 3, str_len(tmp + 2));
                i = -1;
                continue;
            }
            int j = i - 1;
            while (j >= 0 && tmp[j] != '/') j--;
            if (j >= 0) {
                mem_move(tmp + j, tmp + i + 3, str_len(tmp + i));
                i = j - 1;
            }
            continue;
        }
    }
    if (tmp[i - 1] == '/' && i > 1) {
        tmp[i - 1] = '\0';
    }
    str_cpy(path, tmp);
    kfree(tmp);

    return 0;
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

    uint32_t *ret = kmalloc(sizeof(uint32_t) * (filesys->vdisk_count * 3 + 1));
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
