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
    serial_debug("open(%s, %d)\n", path, flags);

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
    return (PROFAN_FNI, -1);
}

int fcntl(int fd, int cmd, ...) {
    serial_debug("fcntl(%d, %d)\n", fd, cmd);

    switch (cmd) {
        case F_DUPFD:
            return (profan_nimpl("fcntl(F_DUPFD)"), -1);
        case F_GETFD:
            return 0;
        case F_SETFD:
            return 0;
        case F_GETFL:
            return (profan_nimpl("fcntl(F_GETFL)"), -1);
        case F_SETFL:
            return (profan_nimpl("fcntl(F_SETFL)"), -1);
        case F_GETLK:
            return (profan_nimpl("fcntl(F_GETLK)"), -1);
        case F_SETLK:
            return (profan_nimpl("fcntl(F_SETLK)"), -1);
        case F_SETLKW:
            return (profan_nimpl("fcntl(F_SETLKW)"), -1);
        default:
            return (profan_nimpl("fcntl(unknown)"), -1);
    }

    return -1;        
}
