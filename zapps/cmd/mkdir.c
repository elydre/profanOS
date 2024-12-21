/*****************************************************************************\
|   === mkdir.c : 2024 ===                                                    |
|                                                                             |
|    Unix command implementation - create a directory              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/stat.h>
#include <stdio.h>

#define MKDIR_USAGE "Usage: mkdir <DIR1> [DIR2] ...\n"

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs(MKDIR_USAGE, stderr);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            fprintf(stderr, "mkdir: invalid option -- '%c'\n"MKDIR_USAGE, argv[i][1]);
            return 1;
        }
    }

    for (int i = 1; i < argc; i++) {
        if (mkdir(argv[i], 0777) == -1) {
            perror("mkdir");
            return 1;
        }
    }

    return 0;
}
