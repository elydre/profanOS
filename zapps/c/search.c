/*****************************************************************************\
|   === search.c : 2024 ===                                                   |
|                                                                             |
|    Basic recursive file search tool (like unix find command)     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <modules/filesys.h>
#include <profan/syscall.h>
#include <profan/carp.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

enum {
    SEARCH_FILE,
    SEARCH_DIR,
    SEARCH_ALL
};

int search_recursive(uint32_t base, const char *path, uint8_t required_type, const char *ext) {
    int offset;

    uint32_t size = syscall_fs_get_size(base);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return 1;
    }

    uint8_t *buf = malloc(size);
    if (syscall_fs_read(base, buf, 0, size)) {
        free(buf);
        return 1;
    }

    for (uint32_t i = 0;; i++) {
        uint32_t sid;
        char *name;

        offset = fu_dir_get_elm(buf, size, i, &sid);

        if (offset <= 0) {
            free(buf);
            return offset == 0 ? 0 : 1;
        }

        name = (char *) buf + offset;

        if (name[0] == '.') {
            continue;
        }

        if (fu_is_file(sid) && ext) {
            char *tmp = strrchr(name, '.');
            if (tmp == NULL || strcmp(tmp + 1, ext) != 0) {
                continue;
            }
        }

        char *p = profan_path_join(path, name);

        if ((required_type == SEARCH_ALL)                    ||
            (required_type == SEARCH_DIR  && fu_is_dir(sid)) ||
            (required_type == SEARCH_FILE && fu_is_file(sid))
        ) puts(p);

        if (fu_is_dir(sid)) {
            if (search_recursive(sid, p, required_type, ext)) {
                free(buf);
                free(p);
                return 1;
            }
        }

        free(p);
    }

    return 0;
}

int main(int argc, char **argv) {
    carp_init("[-f|-d] [-e <ext>] [dir]", 1);

    carp_register('d', CARP_STANDARD, "search for directories only");
    carp_register('e', CARP_NEXT_STR, "search for specific extension");
    carp_register('f', CARP_STANDARD, "search for files only");

    carp_conflict("def");

    if (carp_parse(argc, argv))
        return 1;

    char required_type;

    if (carp_isset('d'))
        required_type = SEARCH_DIR;
    else if (carp_isset('f') || carp_isset('e'))
        required_type = SEARCH_FILE;
    else
        required_type = SEARCH_ALL;

    const char *dir = carp_file_next();
    const char *ext = carp_get_str('e');

    uint32_t base;

    if (dir) {
        base = profan_path_resolve(dir);
    } else {
        base = profan_wd_sid();
        dir = profan_wd_sid() == SID_ROOT ? "/" : ".";
    }

    if (!fu_is_dir(base)) {
        fprintf(stderr, "search: directory '%s' does not exist\n", dir);
        return 1;
    }

    if (search_recursive(base, dir, required_type, ext) == 0) {
        return 0;
    }

    fprintf(stderr, "search: error while listing '%s'\n", dir);
    return 1;
}
