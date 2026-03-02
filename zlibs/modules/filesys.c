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

/********************************************
 *                                         *
 *      Standard directory operations      *
 *                                         *
********************************************/

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

int fu_dir_get_size(sid_t dir_sid) {
    if (!kfu_is_dir(dir_sid))
        return -1;

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t))
        return -2;

    // read the directory
    uint32_t count;

    if (fs_cnt_read(dir_sid, &count, 0, sizeof(uint32_t)))
        return -2;

    return count;
}

int fu_dir_get_content(sid_t dir_sid, sid_t **ids, char ***names) {
    if (!kfu_is_dir(dir_sid))
        return -1;

    sid_t sid;
    int offset;

    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return -2;
    }

    uint8_t *buf = malloc(size);
    if (fs_cnt_read(dir_sid, buf, 0, size)) {
        free(buf);
        return -2;
    }

    // get the number of elements
    uint32_t count;
    count = *(uint32_t *) buf;

    if (count == 0 || ids == NULL || names == NULL) {
        free(buf);
        return count;
    }

    // get the elements
    *ids = malloc(sizeof(sid_t) * count);
    *names = malloc(sizeof(char *) * count);

    for (uint32_t i = 0; i < count; i++) {
        offset = kfu_dir_get_elm(dir_sid, buf, size, i, &sid);
        if (offset <= 0) {
            free(*ids);
            free(*names);
            free(buf);
            return -2;
        }

        (*ids)[i] = sid;
        (*names)[i] = malloc(str_len((char *) buf + offset) + 1);
        str_copy((*names)[i], (char *) buf + offset);
    }

    free(buf);

    return count;
}

int fu_remove_from_dir(sid_t dir_sid, sid_t element_sid) {
    if (!kfu_is_dir(dir_sid))
        return -1;

    // read the directory and get size
    uint32_t size = fs_cnt_get_size(dir_sid);
    if (size == UINT32_MAX)
        return -2;

    // read the directory
    uint8_t *buf = malloc(size);
    if (fs_cnt_read(dir_sid, buf, 0, size))
        return -2;

    uint32_t count = *(uint32_t *) buf;

    if (count == 0) {
        free(buf);
        return -1;
    }

    // get the elements
    sid_t *ids = malloc(sizeof(sid_t) * count);
    uint32_t *name_offsets = malloc(sizeof(uint32_t) * count);

    for (uint32_t i = 0; i < count; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        mem_copy(&ids[i], addr, sizeof(sid_t));
        mem_copy(&name_offsets[i], addr + sizeof(uint32_t), sizeof(uint32_t));
    }

    // search for the element
    uint32_t index = 0;
    for (uint32_t i = 0; i < count; i++) {
        if ((SID_DISK(ids[i]) == SID_DISK(element_sid) && SID_ANONYMIZE(element_sid) == ids[i]) ||
                SID_IS_SAME(ids[i], element_sid) /* mount point */) {
            index = i + 1;
            break;
        }
    }

    if (index == 0) {
        free(name_offsets);
        free(buf);
        free(ids);
        return -1;
    }

    index--;

    for (uint32_t i = index; i < count - 1; i++) {
        void *addr = buf + sizeof(uint32_t) + i * sizeof(uint32_t) * 2;

        ids[i] = ids[i + 1];
        name_offsets[i] = name_offsets[i + 1];

        mem_copy(addr, &ids[i], sizeof(sid_t));
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
        return -2;
    }

    free(buf);
    return 0;
}

/********************************************
 *                                         *
 *        Standard file operations         *
 *                                         *
********************************************/

int fu_file_set_size(sid_t file_sid, uint32_t size) {
    if (!kfu_is_file(file_sid))
        return -1;

    if (fs_cnt_set_size(file_sid, size))
        return -2;

    return 0;
}

uint32_t fu_file_get_size(sid_t file_sid) {
    if (!kfu_is_file(file_sid))
        return UINT32_MAX;

    return fs_cnt_get_size(file_sid);
}

int fu_file_read(sid_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!kfu_is_file(file_sid) || fs_cnt_read(file_sid, buf, offset, size));
}

int fu_file_write(sid_t file_sid, void *buf, uint32_t offset, uint32_t size) {
    return (!kfu_is_file(file_sid) || fs_cnt_write(file_sid, buf, offset, size));
}


void *__module_func_array[] = {
    (void *) 0xF3A3C4D4, // magic
    kfu_is_dir,
    fu_dir_get_size,
    kfu_dir_get_elm,
    fu_dir_get_content,
    kfu_dir_add,
    fu_remove_from_dir,
    kfu_dir_create,
    kfu_is_file,
    fu_file_set_size,
    fu_file_get_size,
    kfu_file_create,
    fu_file_read,
    fu_file_write,
    kfu_is_afft,
    kfu_afft_create,
    kfu_afft_get_id,
    kfu_path_to_sid,
};
