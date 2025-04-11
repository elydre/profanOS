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
#include <stdio.h>
#include <errno.h>

int open(const char *path, int flags, ...) {
    // mode is ignored, permissions are always 777

    char *fullpath;
    fullpath = profan_path_join(profan_wd_path(), path);

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

    int ret = fm_fcntl(fd, cmd, arg);

    if (ret == -0xFFFF) {
        fprintf(stderr, "fcntl(%d, %d) not supported\n", fd, cmd);
        return (PROFAN_FNI, -1);
    }

    if (ret < 0) {
        errno = -ret;
        return -1;
    }
    
    return ret;
}
