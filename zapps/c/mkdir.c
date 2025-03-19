/*****************************************************************************\
|   === mkdir.c : 2025 ===                                                    |
|                                                                             |
|    Unix command implementation - create directories              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/arp.h>

#include <sys/stat.h> // mkdir
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

int mkdir_p(const char *input) {
    char *p, *path;
    int len;

    p = path = malloc((len = strlen(input)) + 2);

    strcpy(path, input);
    if (path[len] != '/')
        path[len++] = '/';
    path[len] = '\0';

    while ((p = strchr(p + 1, '/'))) {
        *p = '\0';
        if (mkdir(path, 0777) == -1 && errno != EEXIST) {
            fprintf(stderr, "mkdir: %s: %m\n", path);
            free(path);
            return 1;
        }
        *p = '/';
    }

    free(path);
    return 0;
}

int main(int argc, char **argv) {
    arp_init("[-p] <dir1> [dir2] ...", ARP_FNOMAX | ARP_FMIN(1));

    arp_register('p', ARP_STANDARD, "make parent directories as needed");

    if (arp_parse(argc, argv))
        return 1;

    const char *path;
    int pflag = arp_isset('p');

    while ((path = arp_file_next())) {
        if (pflag) {
            if (mkdir_p(path) == -1)
                return 1;
            continue;
        }

        if (mkdir(path, 0777) == -1) {
            fprintf(stderr, "mkdir: %s: %m\n", path);
            return 1;
        }
    }

    return 0;
}
