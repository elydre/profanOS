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
#include <profan/syscall.h>
#include <profan.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <errno.h>

static int stat_sid(uint32_t sid, struct stat *buf) {
    if (IS_SID_NULL(sid)) {
        errno = ENOENT;
        return -1;
    }

    buf->st_dev = SID_DISK(sid);
    buf->st_ino = sid;

    buf->st_mode = 0777;

    if (fu_is_dir(sid)) {
        buf->st_mode |= S_IFDIR;
    } else if (fu_is_fctf(sid)) {
        buf->st_mode |= S_IFCHR;
    } else {
        buf->st_mode |= S_IFREG;
    }

    buf->st_nlink = 1;
    buf->st_rdev = 0;   // not a special file

    buf->st_uid = 0;    // root
    buf->st_gid = 0;    // root

    buf->st_size = syscall_fs_get_size(NULL, sid);

    buf->st_atime = 0;
    buf->st_mtime = 0;
    buf->st_ctime = 0;

    buf->st_blksize = 512;
    buf->st_blocks = (buf->st_size + 511) / 512;

    return 0;
}

int chmod(const char *path, mode_t mode) {
    if (IS_SID_NULL(profan_path_resolve(path))) {
        errno = ENOENT;
        return -1;
    }

    // everything is 777 in profanOS :)
    return 0;
}

int fstat(int fd, struct stat *buf) {
    return stat_sid(fm_get_sid(fd), buf);
}

int lstat(const char *path, struct stat *buf) {
    return stat(path, buf);
}

int mknod(const char *path, mode_t mode, dev_t dev) {
    return (PROFAN_FNI, -1);
}

int mkdir(const char *path, mode_t mode) {
    if (!IS_SID_NULL(profan_path_resolve(path))) {
        errno = EEXIST;
        return -1;
    }

    char *fullpath = profan_path_join(profan_wd_path, path);

    if (IS_SID_NULL(fu_dir_create(0, fullpath))) {
        errno = EEXIST;
        free(fullpath);
        return -1;
    }

    free(fullpath);
    return 0;
}

int mkfifo(const char *path, mode_t mode) {
    return (PROFAN_FNI, -1);
}

int stat(const char *path, struct stat *buf) {
    return stat_sid(profan_path_resolve(path), buf);
}

mode_t umask(mode_t mask) {
    return (PROFAN_FNI, 0);
}
