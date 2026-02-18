/*****************************************************************************\
|   === cat.c : 2024 ===                                                      |
|                                                                             |
|    Unix command implementation - concatenates files              .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <modules/filesys.h>
#include <profan/carp.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

int cat_canonical(FILE *file, const char *path) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: %m\n", path);
        return 1;
    }

    char buffer[16];
    int read;
    int offset = 0;
    while ((read = fread(buffer, 1, 16, file)) > 0) {
        printf("%08x  ", offset);
        for (int i = 0; i < 16; i++) {
            if (i < read) {
                printf("%02x ", (uint8_t) buffer[i]);
            } else {
                printf("   ");
            }
        }
        printf(" |");
        for (int i = 0; i < 16; i++) {
            if (i < read) {
                printf("%c", buffer[i] >= 32 && buffer[i] < 127 ? buffer[i] : '.');
            } else {
                printf(" ");
            }
        }
        printf("|\n");
        offset += 16;
    }

    if (file != stdin)
        fclose(file);

    return 0;
}

int cat(FILE *file, const char *path, int end_of_line) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: %m\n", path);
        return 1;
    }

    uint8_t buffer[1024];
    int read;
    while ((read = fread(buffer, 1, 1024, file)) > 0) {
        if (!end_of_line) {
            fwrite(buffer, 1, read, stdout);
            continue;
        }

        for (int i = 0; i < read; i++) {
            if (buffer[i] > 127) {
                putchar('M');
                putchar('-');
                buffer[i] -= 128;
            }
            if (buffer[i] == '\n') {
                putchar('$');
                putchar('\n');
            } else if (isprint(buffer[i])) {
                putchar(buffer[i]);
            } else {
                putchar('^');
                putchar(buffer[i] + 64);
            }
        }
    }

    if (file != stdin)
        fclose(file);

    return 0;
}

int main(int argc, char **argv) {
    carp_init("[options] [file1] [file2] ...", CARP_FNOMAX);

    carp_register('C', CARP_STANDARD, "canonical hex+ASCII display");
    carp_register('e', CARP_STANDARD, "non-printable and $ before newline");

    carp_conflict("Ce");

    if (carp_parse(argc, argv))
        return 1;

    const char **paths = carp_get_files();
    int cat_e = carp_isset('e');
    int ret = 0;

    if (carp_isset('C')) {
        if (paths[0] == NULL)
            ret |= cat_canonical(stdin, "stdin");
        for (int i = 0; paths[i]; i++)
            ret |= cat_canonical(fopen(paths[i], "r"), paths[i]);
    } else {
        if (paths[0] == NULL)
            ret |= cat(stdin, "stdin", cat_e);
        for (int i = 0; paths[i]; i++)
            ret |= cat(fopen(paths[i], "r"), paths[i], cat_e);
    }

    return ret;
}
