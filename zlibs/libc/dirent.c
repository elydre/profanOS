/*****************************************************************************\
|   === dirent.c : 2024 ===                                                   |
|                                                                             |
|    Implementation of dirent functions from libC                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <dirent.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

typedef struct _dirdesc {
    struct dirent *entries;
    int count;
    int pos;
} DIR;

int closedir(DIR *dirp) {
    if (dirp == NULL) {
        errno = EBADF;
        return -1;
    }

    free(dirp);
    return 0;
}

DIR *opendir(const char *dirname) {
    uint32_t dir_sid = profan_resolve_path(dirname);

    uint32_t sid, size;
    uint8_t *buf;

    int i, count = fu_get_dir_size(dir_sid); // OK with SID_NULL

    if (count < 0) {
        errno = -count;
        return NULL;
    }

    size = syscall_fs_get_size(NULL, dir_sid);
    if (size == UINT32_MAX || size < sizeof(uint32_t)) {
        errno = EIO;
        return NULL;
    }

    buf = malloc(size);
    if (syscall_fs_read(NULL, dir_sid, buf, 0, size)) {
        free(buf);
        errno = EIO;
        return NULL;
    }

    DIR *dirp = malloc(sizeof(DIR) + sizeof(struct dirent) * (count + 1));
    dirp->entries = (void *) (dirp + 1);

    for (i = 0; i < count; i++) {
        int offset = fu_get_dir_elm(buf, size, i, &sid);

        if (offset < 0) {
            free(buf);
            errno = -offset;
            return NULL;
        }

        if (offset == 0) {
            break;
        }

        dirp->entries[i].d_ino = sid;
        strlcpy(dirp->entries[i].d_name, (char *) buf + offset, 256);
    }

    free(buf);

    dirp->count = i;
    dirp->pos = 0;

    return dirp;
}

struct dirent *readdir(DIR *dirp) {
    if (dirp == NULL) {
        errno = EBADF;
        return NULL;
    }

    if (dirp->pos >= dirp->count) {
        return NULL;
    }

    return &dirp->entries[dirp->pos++];
}

void rewinddir(DIR *dirp) {
    if (dirp == NULL) {
        errno = EBADF;
        return;
    }

    dirp->pos = 0;
}
