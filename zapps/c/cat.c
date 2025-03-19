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

#include <profan/filesys.h>
#include <profan/arp.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void cat_canonical(FILE *file, const char *path) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: %m\n", path);
        return;
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
}

void cat(FILE *file, const char *path, int end_of_line) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: %m\n", path);
        return;
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

    fclose(file);
}

int main(int argc, char **argv) {
    arp_init("[options] [file1] [file2] ...", ARP_FNOMAX);

    arp_register('C', ARP_STANDARD, "canonical hex+ASCII display");
    arp_register('e', ARP_STANDARD, "non-printable and $ before newline");

    arp_conflict("Ce");

    if (arp_parse(argc, argv))
        return 1;

    const char **paths = arp_get_files();
    int cat_e = arp_isset('e');

    if (arp_isset('C')) {
        if (paths[0] == NULL)
            cat_canonical(stdin, "stdin");
        for (int i = 0; paths[i]; i++)
            cat_canonical(fopen(paths[i], "r"), paths[i]);
    } else {
        if (paths[0] == NULL)
            cat(stdin, "stdin", cat_e);
        for (int i = 0; paths[i]; i++)
            cat(fopen(paths[i], "r"), paths[i], cat_e);
    }

    return 0;
}
