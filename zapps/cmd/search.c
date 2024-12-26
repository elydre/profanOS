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

#include <profan/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define SEARCH_USAGE "Usage: search [-f|-d] [-e <ext>] [dir]\n"

enum {
    SEARCH_FILE,
    SEARCH_DIR,
    SEARCH_ALL
};

int search_recursive(uint32_t base, char *path, uint8_t required_type, char *ext) {
    int offset;

    uint32_t size = syscall_fs_get_size(NULL, base);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        return 1;
    }

    uint8_t *buf = malloc(size);
    if (syscall_fs_read(NULL, base, buf, 0, size)) {
        free(buf);
        return 1;
    }

    for (uint32_t i = 0;; i++) {
        uint32_t sid;
        char *name;

        offset = fu_get_dir_elm(buf, size, i, &sid);

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

        char *p = profan_join_path(path, name);

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
    (void) argc;

    char required_type = SEARCH_ALL;
    char *dir = NULL;
    char *ext = NULL;

    for (int i = 1; argv[i] != NULL; i++) {
        if (argv[i][0] != '-') {
            if (dir == NULL) {
                dir = argv[i];
                continue;
            }
            fprintf(stderr, "search: Directory already set to '%s'\n" SEARCH_USAGE, dir);
            return 1;
        }

        if (argv[i][1] == '-') {
            fprintf(stderr, "search: Unknown option '%s'\n" SEARCH_USAGE, argv[i]);
            return 1;
        }

        if (argv[i][1] == 'f') {
            required_type = SEARCH_FILE;
        } else if (argv[i][1] == 'd') {
            if (ext != NULL) {
                fprintf(stderr, "search: Cannot use -e with -d\n" SEARCH_USAGE);
                return 1;
            }
            required_type = SEARCH_DIR;
        } else if (argv[i][1] == 'e') {
            if (required_type == SEARCH_DIR) {
                fprintf(stderr, "search: Cannot use -d with -e\n" SEARCH_USAGE);
                return 1;
            }
            if (argv[++i]) {
                ext = argv[i];
                required_type = SEARCH_FILE;
                continue;
            }
            fprintf(stderr, "search: Missing argument for -e\n" SEARCH_USAGE);
            return 1;
        } else {
            fprintf(stderr, "search: Unknown argument -- '%c'\n" SEARCH_USAGE, argv[i][1]);
            return 1;
        }
    }

    uint32_t base;

    if (dir) {
        base = profan_resolve_path(dir);
    } else {
        base = profan_wd_sid;
        dir = profan_wd_sid == SID_ROOT ? "/" : ".";
    }

    if (!fu_is_dir(base)) {
        fprintf(stderr, "search: Directory '%s' does not exist\n" SEARCH_USAGE, dir);
        return 1;
    }

    if (search_recursive(base, dir, required_type, ext) == 0) {
        return 0;
    }

    fprintf(stderr, "search: Error while listing '%s'\n", dir);
    return 1;
}
