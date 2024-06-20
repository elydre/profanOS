/*****************************************************************************\
|   === less.c : 2024 ===                                                     |
|                                                                             |
|    Unix command implementation - display file line by line       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK SHARED: libtsi

#include <profan/libtsi.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *read_file(char *filename) {
    // read from file
    FILE *fp = fopen(filename, "r");
    if (fp == NULL)
        return NULL;

    char *buffer = malloc(1024);
    int buffer_size = 0;
    int rcount = 0;

    while ((rcount = fread(buffer + buffer_size, 1, 1024, fp)) > 0) {
        buffer = realloc(buffer, buffer_size + rcount + 1025);
        buffer_size += rcount;
    }

    buffer[buffer_size] = '\0';

    fclose(fp);

    return buffer;
}

int main(int argc, char *argv[]) {
    char *buffer, *filename, *title;

    if (argc > 2 || (argc == 2 && argv[1][0] == '-')) {
        fputs("Usage: more [file]", stderr);
        return 1;
    }

    if (argc < 2 && isatty(STDIN_FILENO)) {
        fputs("more: stdin is a tty", stderr);
        return 1;
    }

    filename = argc == 2 ? argv[1] : "/dev/stdin";
    buffer = read_file(filename);

    if (buffer == NULL) {
        fprintf(stderr, "more: %s: File not found\n", filename);
        return 1;
    }

    title = malloc(strlen(filename) + 8);
    sprintf(title, "less: %s", filename);

    tsi_start(title, buffer);

    free(buffer);
    free(title);

    return 0;
}

