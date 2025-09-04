/*****************************************************************************\
|   === utsname.c : 2025 ===                                                  |
|                                                                             |
|    Implementation of sys/utsname function from libC              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int uname(struct utsname *buf) {
    // getline from /sys/kernel/version.txt

    FILE *f = fopen("/sys/kernel/version.txt", "r");
    if (f == NULL) {
        errno = ENOENT;
        return -1;
    }

    char *info = NULL;
    size_t len = 0;

    char buffers[3][65]; // version, release, machine

    for (int i = 0; i < 3; i++) {
        ssize_t read = getline(&info, &len, f);

        if (read == -1) {
            fclose(f);
            free(info);
            errno = EIO;
            return -1;
        }

        // Remove newline character if present
        if (info[read - 1] == '\n') {
            info[read - 1] = '\0';
        }

        strlcpy(buffers[i], info, sizeof(buffers[i]));
    }

    strcpy(buf->version, buffers[0]);
    strcpy(buf->release, buffers[1]);
    strcpy(buf->machine, buffers[2]);

    fclose(f);
    free(info);
   
    char *sysname = getenv("UNAME_KERNEL");

    if (sysname == NULL) {
        strcpy(buf->sysname, "profan");
        strcpy(buf->nodename, "( ._.)");
    } else {
        strlcpy(buf->sysname, sysname, sizeof(buf->sysname));
        strcpy(buf->nodename, "( ^-^)");
    }

    return 0;
}
