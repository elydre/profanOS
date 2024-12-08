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

    free(dirp->entries);
    free(dirp);

    return 0;
}

DIR *opendir(const char *dirname) {
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";
    char *fullpath = profan_join_path(pwd, dirname);

    uint32_t dir_sid = fu_path_to_sid(SID_ROOT, fullpath);

    free(fullpath);

    if (!fu_is_dir(dir_sid)) {
        errno = ENOENT;
        return NULL;
    }

    uint32_t *sids;
    char **names;

    int count = fu_get_dir_content(dir_sid, &sids, &names);

    if (count == -1) {
        errno = EIO;
        return NULL;
    }

    DIR *dirp = malloc(sizeof(DIR));
    dirp->entries = malloc(sizeof(struct dirent) * count);

    for (int i = 0; i < count; i++) {
        dirp->entries[i].d_ino = sids[i];
        strncpy(dirp->entries[i].d_name, names[i], 255);
        dirp->entries[i].d_name[255] = '\0';
    }

    dirp->count = count;
    dirp->pos = 0;

    for (int i = 0; i < count; i++)
        profan_kfree(names[i]);
    profan_kfree(names);
    profan_kfree(sids);

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
