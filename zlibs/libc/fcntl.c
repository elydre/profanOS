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
#include <unistd.h>
#include <stdarg.h>
#include <fcntl.h>
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
    return (PROFAN_FNI, -1);
}

int fcntl(int fd, int cmd, ...) {
    va_list ap;
    va_start(ap, cmd);
    int arg = va_arg(ap, int);
    va_end(ap);

    switch (cmd) {
        case F_DUPFD:
            int new_fd = fm_newfd_after(arg);
            serial_debug("fcntl(F_DUPFD, %d) -> %d\n", arg, new_fd);

            if (new_fd < 0) {
                errno = -new_fd;
                return -1;
            }

            return dup2(fd, new_fd);
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
