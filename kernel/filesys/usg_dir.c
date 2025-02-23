/*****************************************************************************\
|   === usg_dir.c : 2024 ===                                                  |
|                                                                             |
|    Kernel-only directory manipulation functions                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

int fu_is_dir(filesys_t *filesys, uint32_t dir_sid) {
    if (IS_SID_NULL(dir_sid)) return 0;
    char *name = fs_cnt_meta(filesys, dir_sid, NULL);
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

int fu_get_dir_content(filesys_t *filesys, uint32_t dir_sid, uint32_t **ids, char ***names) {
    if (!fu_is_dir(filesys, dir_sid)) {
        sys_warning("[get_dir_content] Sector is not a directory");
        return -1;
    }

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        return -1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (fs_cnt_read(filesys, dir_sid, buf, 0, size)) {
        return -1;
    }

    // get the number of elements
    uint32_t count;
    mem_copy(&count, buf, sizeof(uint32_t));

    if (count == 0 || ids == NULL || names == NULL) {
        free(buf);
        return count;
    }

    // get the elements
    *ids = malloc(sizeof(uint32_t) * count);
    *names = malloc(sizeof(char *) * count);

    if (*ids == NULL || *names == NULL) {
        return -1;
    }

    uint32_t name_offset;
    for (uint32_t i = 0; i < count; i++) {
        mem_copy(*ids + i, buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)), sizeof(uint32_t));
        mem_copy(&name_offset, buf + sizeof(uint32_t) + i * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t),
                sizeof(uint32_t));
        char *tmp = (void *) buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)) + name_offset;
        (*names)[i] = malloc(str_len(tmp) + 1);
        str_cpy((*names)[i], tmp);
    }
    free(buf);

    return count;
}

int fu_add_element_to_dir(filesys_t *filesys, uint32_t dir_sid, uint32_t element_sid, char *name) {
    if (IS_SID_NULL(element_sid) || IS_SID_NULL(dir_sid) || !fu_is_dir(filesys, dir_sid)) {
        sys_error("[add_element_to_dir] Invalid given sector id");
        return 1;
    }

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        return 1;
    }

    // extend the directory
    if (fs_cnt_set_size(filesys, dir_sid, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1)) {
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1);
    if (fs_cnt_read(filesys, dir_sid, buf, 0, size)) {
        return 1;
    }

    uint32_t count = 0;
    // get the number of elements
    mem_copy(&count, buf, sizeof(uint32_t));

    if (count > 0) {
        // move names
        for (uint32_t i = size - 1; i >= sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)); i--) {
            buf[i + sizeof(uint32_t) + sizeof(uint32_t)] = buf[i];
        }
    }

    // insert the new element
    mem_copy(buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)), &element_sid, sizeof(uint32_t));
    uint32_t name_offset = size - sizeof(uint32_t) - count * (sizeof(uint32_t) + sizeof(uint32_t));
    mem_copy(buf + sizeof(uint32_t) + count * (sizeof(uint32_t) + sizeof(uint32_t)) + sizeof(uint32_t), &name_offset,
            sizeof(uint32_t));

    // update the number of elements
    count++;
    mem_copy(buf, &count, sizeof(uint32_t));

    str_cpy((char *) buf + sizeof(uint32_t) + count * sizeof(uint32_t) + count * sizeof(uint32_t) + name_offset, name);

    // write the directory
    if (fs_cnt_write(filesys, dir_sid, buf, 0, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1)) {
        return 1;
    }

    free(buf);

    return 0;
}

uint32_t fu_dir_create(filesys_t *filesys, uint8_t device_id, char *path) {
    char *parent, *name;

    uint32_t parent_sid;
    uint32_t head_sid;

    sep_path(path, &parent, &name);
    if (parent[0]) {
        parent_sid = fu_path_to_sid(filesys, SID_ROOT, parent);
        if (IS_SID_NULL(parent_sid)) {
            sys_warning("[dir_create] Parent not found");
            free(parent);
            free(name);
            return SID_NULL;
        }

        if (!fu_is_dir(filesys, parent_sid)) {
            sys_warning("[dir_create] Parent not a directory");
            free(parent);
            free(name);
            return SID_NULL;
        }
    } else {
        parent_sid = SID_ROOT;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "D-");
    str_ncpy(meta + 1, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init(filesys, (device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid)) {
        free(parent);
        free(name);
        return SID_NULL;
    }

    // create a link in parent directory
    if (parent[0]) {
        if (fu_add_element_to_dir(filesys, parent_sid, head_sid, name)) {
            free(parent);
            free(name);
            return SID_NULL;
        }
    }

    fs_cnt_set_size(filesys, head_sid, sizeof(uint32_t));
    fs_cnt_write(filesys, head_sid, "\0\0\0\0", 0, 4);

    // create '.' and '..'
    if (fu_add_element_to_dir(filesys, head_sid, parent_sid, "..")) {
        free(parent);
        free(name);
        return SID_NULL;
    }

    if (fu_add_element_to_dir(filesys, head_sid, head_sid, ".")) {
        free(parent);
        free(name);
        return SID_NULL;
    }

    free(parent);
    free(name);

    return head_sid;
}
