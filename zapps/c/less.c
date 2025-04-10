/*****************************************************************************\
|   === less.c : 2024 ===                                                     |
|                                                                             |
|    Unix command implementation - show file with libtsi           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/libtsi.h>
#include <profan/cap.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define HELP_MSG "Try 'less -h' for more information.\n"

char *read_file(FILE *fp) {
    char *buffer = malloc(1024);
    int buffer_size = 0;
    int rcount = 0;

    while ((rcount = fread(buffer + buffer_size, 1, 1024, fp)) > 0) {
        buffer = realloc(buffer, buffer_size + rcount + 1025);
        buffer_size += rcount;
    }

    buffer[buffer_size] = '\0';

    return buffer;
}

int main(int argc, char **argv) {
    cap_init("[-pn] [-t title] [file]", 1);

    cap_register('p', CAP_STANDARD, "block non-printable characters");
    cap_register('n', CAP_STANDARD, "disable long lines wrapping");
    cap_register('t', CAP_NEXT_STR, "set TSI interface title");

    if (cap_parse(argc, argv))
        return 1;

    uint32_t flags = 0;

    if (!cap_isset('p'))
        flags |= TSI_NON_PRINTABLE;
    if (cap_isset('n'))
        flags |= TSI_NO_AUTO_WRAP;

    const char *filename = cap_file_next();
    FILE *fp = filename ? fopen(filename, "r") : stdin;
    filename = filename ? filename : "stdin";

    if (fp == NULL) {
        fprintf(stderr, "less: %s: %m\n", filename);
        return 1;
    }

    const char *title = cap_isset('t') ? cap_get_str('t') : filename;
    char *buffer = read_file(fp);

    tsi_start(title, buffer, flags);

    if (fp != stdin)
        fclose(fp);

    free(buffer);

    return 0;
}
