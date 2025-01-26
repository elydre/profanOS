/*****************************************************************************\
|   === strings.c : 2025 ===                                                  |
|                                                                             |
|    Unix command implementation - print strings in raw file       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

int g_min_length = 4;

void print_strings(FILE *file) {
    int first = -1;
    int c;

    for (int j = 0; (c = fgetc(file)) != EOF; j++) {
        if (isprint(c)) {
            if (first == -1)
                first = j;
        } else if (first != -1) {
            if (j - first >= g_min_length) {
                fseek(file, first, SEEK_SET);
                for (int i = first; i < j; i++)
                    putchar(fgetc(file));
                putchar('\n');
                fgetc(file);
            }
            first = -1;
        }
    }
}

int main(int argc, char **argv) {
    if (argc < 2) {
        fputs("Usage: strings [-n <int>] [file1] [file2] ...\n", stderr);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-')
            continue;
        if (strlen(argv[i]) != 2) {
            fprintf(stderr, "strings: unrecognized option -- '%s'\n", argv[i]);
        } else if (argv[i][1] == 'n') {
            if (i + 1 >= argc) {
                fputs("strings: missing argument to '-n'\n", stderr);
                return 1;
            }

            g_min_length = atoi(argv[i + 1]);
            if (g_min_length < 1) {
                fputs("strings: invalid minimum string length\n", stderr);
                return 1;
            }

            continue;
        } else {
            fprintf(stderr, "strings: invalid option -- '%c'\n", argv[i][1]);
        }
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            i++;
            continue;
        }

        FILE *file = fopen(argv[i], "r");

        if (file == NULL) {
            fprintf(stderr, "strings: %s: %m\n", argv[i]);
            return 1;
        }

        print_strings(file);
        fclose(file);
    }

    return 0;
}
