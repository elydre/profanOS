/*****************************************************************************\
|   === stat.c : 2024 ===                                                     |
|                                                                             |
|    Implementation of sys/stat functions from libC                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

int chmod(const char *path, mode_t mode) {
    if (IS_SID_NULL(profan_resolve_path(path))) {
        errno = ENOENT;
        return -1;
    }

    // everything is 777 in profanOS :)
    return 0;
}
int fstat(int fd, struct stat *buf) {
    profan_nimpl("fstat");
    return -1;
}

int mknod(const char *path, mode_t mode, dev_t dev) {
    profan_nimpl("mknod");
    return -1;
}

int mkdir(const char *path, mode_t mode) {
    if (!IS_SID_NULL(profan_resolve_path(path))) {
        errno = EEXIST;
        return -1;
    }

    char *fullpath = profan_join_path(profan_wd_path, path);

    if (IS_SID_NULL(fu_dir_create(0, fullpath))) {
        errno = EEXIST;
        free(fullpath);
        return -1;
    }

    free(fullpath);
    return 0;
}

int mkfifo(const char *path, mode_t mode) {
    profan_nimpl("mkfifo");
    return -1;
}

int stat(const char *path, struct stat *buf) {
    if (IS_SID_NULL(profan_resolve_path(path))) {
        errno = ENOENT;
        return -1;
    }

    return 0;
}

mode_t umask(mode_t mask) {
    profan_nimpl("umask");
    return 0;
}
