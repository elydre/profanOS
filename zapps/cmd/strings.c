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

#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <ctype.h>

void print_strings(FILE *file) {
    int first = -1;
    int c;

    for (int j = 0; (c = fgetc(file)) != EOF; j++) {
        if (isprint(c)) {
            if (first == -1)
                first = j;
        } else if (first != -1) {
            if (j - first > 3) {
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
        fputs("Usage: strings [file1] [file2] ...\n", stderr);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        FILE *file = fopen(argv[i], "r");

        if (file == NULL) {
            fprintf(stderr, "strings: %s: %s\n", argv[i], strerror(errno));
            return 1;
        }

        print_strings(file);
        fclose(file);
    }

    return 0;
}
