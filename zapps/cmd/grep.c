/*****************************************************************************\
|   === grep.c : 2024 ===                                                     |
|                                                                             |
|    Unix command implementation - search pattern in file          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

void print_with_color(char *line, char *pattern) {
    char *pattern_start, *pattern_end;

    if ((pattern_start = strstr(line, pattern)) == NULL) {
        fputs(line, stdout);
        return;
    }

    fwrite(line, 1, pattern_start - line, stdout);
    pattern_end = pattern_start + strlen(pattern);

    fputs("\e[31m", stdout);
    fwrite(pattern_start, 1, pattern_end - pattern_start, stdout);
    fputs("\e[0m", stdout);

    print_with_color(pattern_end, pattern);
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: <CMD> | %s <pattern>\n", argv[0]);
        return 1;
    }

    int color = isatty(1);

    size_t s;
    char *line = NULL;

    while (getline(&line, &s, stdin) != -1) {
        if (strstr(line, argv[1]) == NULL)
            continue;
        if (color)
            print_with_color(line, argv[1]);
        else
            fputs(line, stdout);
    }

    free(line);

    return 0;
}
