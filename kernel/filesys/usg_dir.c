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

int kfu_is_dir(uint32_t dir_sid) {
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

int kfu_get_dir_content(uint32_t dir_sid, uint32_t **ids, char ***names) {
    if (!kfu_is_dir(dir_sid)) {
        sys_warning("[get_dir_content] Sector is not a directory");
        return -1;
    }

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX) {
        return -1;
    }

    // read the directory
    uint8_t *buf = malloc(size);
    if (fs_cnt_read(dir_sid, buf, 0, size)) {
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

int kfu_add_element_to_dir(uint32_t dir_sid, uint32_t element_sid, const char *name) {
    if (IS_SID_NULL(element_sid) || !kfu_is_dir(dir_sid)) {
        sys_error("[add_element_to_dir] Invalid given sector id");
        return 1;
    }

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX) {
        return 1;
    }

    // extend the directory
    if (fs_cnt_set_size(dir_sid, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1)) {
        return 1;
    }

    // read the directory
    uint8_t *buf = malloc(size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1);
    if (fs_cnt_read(dir_sid, buf, 0, size)) {
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
    if (fs_cnt_write(dir_sid, buf, 0, size + sizeof(uint32_t) + sizeof(uint32_t) + str_len(name) + 1)) {
        return 1;
    }

    free(buf);

    return 0;
}

uint32_t kfu_dir_create(uint8_t device_id, const char *parent, const char *name) {
    uint32_t parent_sid;
    uint32_t head_sid;

    if (parent) {
        parent_sid = kfu_path_to_sid(SID_ROOT, parent);
        if (!kfu_is_dir(parent_sid)) {
            sys_warning("[dir_create] Parent unreachable");
            return SID_NULL;
        }
    } else {
        parent_sid = SID_ROOT;
    }

    // generate the meta
    char *meta = malloc(META_MAXLEN);
    str_cpy(meta, "D-");
    str_ncpy(meta + 2, name, META_MAXLEN - 3);

    head_sid = fs_cnt_init((device_id > 0) ? (uint32_t) device_id : SID_DISK(parent_sid), meta);
    free(meta);

    if (IS_SID_NULL(head_sid))
        return SID_NULL;

    // create a link in parent directory
    if (parent && kfu_add_element_to_dir(parent_sid, head_sid, name))
        return SID_NULL;

    fs_cnt_set_size(head_sid, sizeof(uint32_t));
    fs_cnt_write(head_sid, "\0\0\0\0", 0, 4);

    // create '.' and '..'
    return (kfu_add_element_to_dir(head_sid, parent_sid, "..") ||
            kfu_add_element_to_dir(head_sid, head_sid, ".") ? SID_NULL : head_sid);
}
