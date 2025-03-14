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

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#define HELP_MSG "Try 'less -h' for more information.\n"

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

char *filename;

uint32_t compute_args(int argc, char **argv) {
    uint32_t flags = TSI_NON_PRINTABLE;
    filename = "/dev/stdin";

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            if (i < argc - 1) {
                fprintf(stderr, "less: too many arguments\n"HELP_MSG);
                exit(1);
            }
            filename = argv[i];
            break;
        }

        if (argv[i][1] == '\0' || argv[i][1] == '-' || argv[i][2] != '\0') {
            fprintf(stderr, "less: unrecognized option '%s'\n" HELP_MSG, argv[i]);
            exit(1);
        }

        switch (argv[i][1]) {
            case 'n':
                flags |= TSI_NO_AUTO_WRAP;
                break;
            case 'p':
                flags &= ~TSI_NON_PRINTABLE;
                break;
            case 'h':
                puts("Usage: less [flags] [file]");
                puts("Options:\n"
                    "  -n   disable long lines wrapping\n"
                    "  -p   block non-printable characters\n"
                    "  -h   display this help message");
                return (exit(0), 0);
            default:
                fprintf(stderr, "less: invalid option -- '%c'\n"HELP_MSG, argv[i][1]);
                exit(1);
        }
    }

    return flags;
}

int main(int argc, char **argv) {
    char *buffer, *title;

    uint32_t flags = compute_args(argc, argv);

    buffer = read_file(filename);

    if (buffer == NULL) {
        fprintf(stderr, "less: %s: File not found\n", filename);
        return 1;
    }

    title = malloc(strlen(filename) + 8);
    sprintf(title, "less: %s", filename);

    tsi_start(title, buffer, flags);

    free(buffer);
    free(title);

    return 0;
}
