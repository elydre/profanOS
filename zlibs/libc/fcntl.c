/*****************************************************************************\
|   === fcntl.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of fcntl functions from libC                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdio.h>

int open(const char *path, int flags, ...) {
    // mode is ignored, permissions are always 777

    char *fullpath, *cwd = getenv("PWD");
    fullpath = cwd ? profan_join_path(cwd, path) : strdup(path);

    uint32_t sid = fu_path_to_sid(SID_ROOT, fullpath);

    if (IS_SID_NULL(sid) && (flags & O_CREAT)) {
        sid = fu_file_create(0, fullpath);
    }

    if (IS_SID_NULL(sid)) {
        free(fullpath);
        return -1;
    }

    if (flags & O_TRUNC && fu_is_file(sid)) {
        fu_file_set_size(sid, 0);
    }

    int fd = fm_open(fullpath);

    if (fd < 0) {
        free(fullpath);
        return -1;
    }

    if (flags & O_APPEND) {
        fm_lseek(fd, 0, SEEK_END);
    }

    free(fullpath);
    return fd;
}

// compatibility for old precompiled programs
int profan_open(const char *path, int flags, ...) {
    return open(path, flags);
}

int creat(const char *file, mode_t mode) {
    puts("creat is not implemented yet, WHY DO YOU USE IT ?");
    return -1;
}

int fcntl(int fd, int cmd, ...) {
    puts("fcntl is not implemented yet, WHY DO YOU USE IT ?");
    return -1;
}
