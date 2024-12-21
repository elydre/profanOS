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
#include <errno.h>

int open(const char *path, int flags, ...) {
    // mode is ignored, permissions are always 777

    char *fullpath;
    fullpath = profan_join_path(profan_wd_path, path);

    int fd = fm_open(fullpath, flags);
    free(fullpath);

    if (fd >= 0) {
        return fd;
    }

    errno = -fd;
    return -1;
}

int creat(const char *file, mode_t mode) {
    profan_nimpl("creat");
    return -1;
}

int fcntl(int fd, int cmd, ...) {
    profan_nimpl("fcntl");
    return -1;
}
