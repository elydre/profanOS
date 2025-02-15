/*****************************************************************************\
|   === rev.c : 2024 ===                                                      |
|                                                                             |
|    Unix command implementation - reverse lines from stdin        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

int main(int argc) {
    if (argc != 1) {
        fputs("Usage: <CMD> | rev\n", stderr);
        return 1;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, stdin)) != -1) {
        for (ssize_t i = read - 1; i >= 0; i--) {
            if (line[i] == '\n')
                continue;
            putchar(line[i]);
        }
        putchar('\n');
    }

    free(line);

    return 0;
}
