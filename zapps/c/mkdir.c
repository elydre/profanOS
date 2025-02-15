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

#include <sys/stat.h> // mkdir
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

#define MKDIR_USAGE "Usage: mkdir [-p] <DIR1> [DIR2] ...\n"

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
            perror("mkdir");
            free(path);
            return 1;
        }
        *p = '/';
    }

    free(path);
    return 0;
}

int main(int argc, char **argv) {
    int pflag = 0;

    if (argc > 1 && strcmp(argv[1], "-p") == 0)
        pflag = 1;

    if (argc < 2 + pflag)
        return (fputs(MKDIR_USAGE, stderr), 1);

    for (int i = 1 + pflag; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] != '-')
                fprintf(stderr, "mkdir: invalid option -- '%c'\n" MKDIR_USAGE, argv[i][1]);
            else
                fprintf(stderr, "mkdir: unrecognized option -- '%s'\n" MKDIR_USAGE, argv[i]);
            return 1;
        }

        if (pflag) {
            if (mkdir_p(argv[i]) == -1)
                return 1;
            continue;
        }

        if (mkdir(argv[i], 0777) == -1) {
            perror("mkdir");
            return 1;
        }
    }

    return 0;
}
