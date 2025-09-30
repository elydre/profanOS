/*****************************************************************************\
|   === usg_tools.c : 2025 ===                                                |
|                                                                             |
|    Part of the filesystem creation tool                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <minilib.h>
#include <system.h>

static inline int path_to_sid_cmp(const char *path, const char *name, uint32_t len) {
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
        return SID_NULL;

    buf = malloc(size);

    if (fs_cnt_read(parent, buf, 0, size)) {
        free(buf);
        return SID_NULL;
    }

    // search for the path part
    for (int j = 0; (offset = kfu_dir_get_elm(buf, size, j, &sid)) > 0; j++) {
        if (str_cmp(path, (char *) buf + offset) == 0) {
            free(buf);
            return sid;
        }

        if (path_to_sid_cmp(path, (char *) buf + offset, path_len) == 0 && kfu_is_dir(sid)) {
            free(buf);
            return rec_path_to_sid(sid, path + path_len);
        }
    }

    free(buf);

    return SID_NULL;
}

uint32_t kfu_path_to_sid(uint32_t from, const char *path) {
    uint32_t ret;

    if (str_cmp("/", path) == 0)
        return from;

    ret = rec_path_to_sid(from, path);

    return ret;
}

void kfu_sep_path(const char *fullpath, char **parent, char **cnt) {
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
            str_copy(*parent, "/");
        } else {
            str_ncopy(*parent, fullpath, i);
        }
    }

    if (cnt != NULL) {
        str_copy(*cnt, fullpath + i + 1);
    }
}

void kfu_draw_tree(sid_t sid, int depth) {
    sid_t *sids;
    char **names;
    int count;

    count = kfu_dir_get_content(sid, &sids, &names);

    if (count == 0)
        return;

    if (count == -1) {
        kprintf_serial("failed to get directory content during path search\n");
        return;
    }

    for (int i = 0; i < count; i++) {
        if (str_cmp(names[i], ".") == 0 || str_cmp(names[i], "..") == 0)
            continue;

        kprintf_serial("%08x  ", sids[i]);

        for (int j = 0; j < depth; j++)
            kprintf_serial("  ");

        kprintf_serial("%s : %dB\n", names[i], fs_cnt_get_size(sids[i]));

        if (kfu_is_dir(sids[i]))
            kfu_draw_tree(sids[i], depth + 1);
    }

    for (int i = 0; i < count; i++)
        free(names[i]);

    free(names);
    free(sids);
}

void kfu_dump_sector(sid_t sid) {
    uint8_t buf[SECTOR_SIZE];
    if (vdisk_read(buf, SECTOR_SIZE, SID_SECTOR(sid) * SECTOR_SIZE)) {
        kprintf_serial("failed to read sector d%ds%d\n", SID_DISK(sid), SID_SECTOR(sid));
        return;
    }

    kprintf_serial("SECTOR d%ds%d:\n", SID_DISK(sid), SID_SECTOR(sid));

    char line[16];

    for (int i = 0; i < SECTOR_SIZE; i++) {
        if (i % 16 == 0)
            kprintf_serial("%04x: ", i);
        kprintf_serial("%02x ", buf[i]);
        line[i % 16] = (buf[i] >= 32 && buf[i] <= 126) ? buf[i] : '.';
        if (i % 16 == 15) {
            line[16] = 0;
            kprintf_serial("  %s\n", line);
        }
    }
}
