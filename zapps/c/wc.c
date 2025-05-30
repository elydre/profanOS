/*****************************************************************************\
|   === wc.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - count from stdin                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/carp.h>

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

typedef struct {
    int lines;
    int words;
    int bytes;
} wc_t;

void wc_file(FILE *f, wc_t *wc) {
    int c, in_word = 0;

    wc->lines = wc->words = wc->bytes = 0;

    while ((c = fgetc(f)) != EOF) {
        wc->bytes++;
        if (c == '\n') {
            wc->lines++;
        }
        if (c == ' ' || c == '\n' || c == '\t') {
            in_word = 0;
        } else if (!in_word) {
            in_word = 1;
            wc->words++;
        }
    }
}

void wc_print(wc_t *wc, wc_t *format, const char *file) {
    int sum = format->lines + format->words + format->bytes;
    if (sum == 0)
        format->lines = format->words = format->bytes = 1;

    if (sum == 1 || file) {
        if (file)
            printf("%s: ", file);
        if (format->lines)
            printf("%d", wc->lines);
        if (format->words) {
            if (format->lines)
                putchar(' ');
            printf("%d", wc->words);
        }
        if (format->bytes) {
            if (format->lines || format->words)
                putchar(' ');
            printf("%d", wc->bytes);
        }
        putchar('\n');
        return;
    }

    if (format->lines)
        printf("%d lines\n", wc->lines);
    if (format->words)
        printf("%d words\n", wc->words);
    if (format->bytes)
        printf("%d bytes\n", wc->bytes);
}

int main(int argc, char **argv) {
    carp_init("[options] [file1] [file2] ...", CARP_FNOMAX);

    carp_register('l', CARP_STANDARD, "print newline count");
    carp_register('w', CARP_STANDARD, "print word count");
    carp_register('c', CARP_STANDARD, "print byte count");

    if (carp_parse(argc, argv))
        return 1;

    wc_t wc, format = {
        .lines = carp_isset('l'),
        .words = carp_isset('w'),
        .bytes = carp_isset('c')
    };

    int file_count = carp_file_count();
    const char *file;

    if (file_count == 0) {
        wc_file(stdin, &wc);
        wc_print(&wc, &format, NULL);
        return 0;
    }

    while ((file = carp_file_next())) {
        FILE *f = fopen(file, "r");
        if (f == NULL) {
            perror(file);
            continue;
        }
        wc_file(f, &wc);
        wc_print(&wc, &format, file_count > 1 ? file : NULL);
        fclose(f);
    }

    return 0;
}
