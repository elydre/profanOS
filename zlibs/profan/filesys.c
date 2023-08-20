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

/********************************************
 *                                         *
 *          Auxiliary functions            *
 *                                         *
********************************************/

// init function
int main(void) {
    return 0;
}

void sep_path(char *fullpath, char **parent, char **cnt) {
    int i, len;

    *parent = (char *) malloc(META_MAXLEN);
    *cnt = (char *) malloc(META_MAXLEN);

    len = strlen(fullpath);
    if (len == 0 || (len == 1 && fullpath[0] == '/')) {
        (*parent)[0] = '\0';
        strcpy((*cnt), "/");
        return;
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

    // read the directory and get size
    uint32_t size = c_fs_cnt_get_size(filesys, dir_sid);
    if (size == UINT32_MAX) {
        printf("failed to get directory size\n");
        return -1;
    }

    if (!fu_is_dir(dir_sid)) {
        printf("not a directory\n");
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

sid_t fu_dir_create(int device_id, char *path) {
    filesys_t *filesys = c_fs_get_main();

    char *parent, *name;

    sid_t parent_sid;
    sid_t head_sid;

    sep_path(path, &parent, &name);
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

    sep_path(path, &parent, &name);
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
        ret = ROOT_SID;
    } else if (path[0] == '/') {
        ret = fu_rec_path_to_sid(filesys, from, path + 1);
    } else {
        ret = fu_rec_path_to_sid(filesys, from, path);
    }

    if (IS_NULL_SID(ret)) {
        return NULL_SID;
    }

    return ret;
}
