/*****************************************************************************\
|   === dirent.h : 2024 ===                                                   |
|                                                                             |
|    Implementation of the dirent.h header file from libC          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#ifndef _DIRENT_H
#define _DIRENT_H

#include <sys/types.h>

struct dirent {
    ino_t   d_ino;
    char    d_name[256];
};

typedef struct _dirdesc DIR;

int            closedir(DIR *dirp);
DIR           *opendir(const char *dirname);
struct dirent *readdir(DIR *dirp);
void           rewinddir(DIR *dirp);

#endif
