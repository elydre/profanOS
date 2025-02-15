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

#include <profan/filesys.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>

void show_help(void) {
    puts("Usage: cat [options] [files]\n"
        "Options:\n"
        "  -C       canonical hex+ASCII display\n"
        "  -e       non-printable and $ before newline\n"
        "  -h       display this help message"
    );
}

typedef struct {
    int canonical;
    int help;
    int end_of_line;
    int failed;
    char **paths;
} cat_args_t;

cat_args_t *parse_args(int argc, char **argv) {
    cat_args_t *args = malloc(sizeof(cat_args_t));
    args->canonical = 0;
    args->end_of_line = 0;
    args->help = 0;
    args->failed = 0;
    args->paths = calloc(argc, sizeof(char *));

    int path_count = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-' && argv[i][1] != '\0') {
            if (strcmp(argv[i], "-C") == 0) {
                args->canonical = 1;
            } else if (strcmp(argv[i], "-e") == 0) {
                args->end_of_line = 1;
            } else if (strcmp(argv[i], "-h") == 0) {
                args->help = 1;
            } else {
                fprintf(stderr, "cat: invalid option -- '%s'\n", argv[i] + 1);
                args->failed = 1;
                return args;
            }
        } else {
            args->paths[path_count++] = argv[i];
        }
    }

    return args;
}

void free_args(cat_args_t *args) {
    free(args->paths);
    free(args);
}

void cat_canonical(FILE *file, char *path) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: File not found\n", path);
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

void cat(FILE *file, char *path, int end_of_line) {
    if (file == NULL) {
        fprintf(stderr, "cat: %s: File not found\n", path);
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
    cat_args_t *args = parse_args(argc, argv);
    if (args->failed) {
        fputs("Try 'cat -h' for more information.\n", stderr);
        free_args(args);
        return 1;
    }

    if (args->help) {
        show_help();
        free_args(args);
        return 0;
    }

    if (args->end_of_line && args->canonical) {
        fputs("cat: Cannot use -e and -c together\n", stderr);
        free_args(args);
        return 1;
    }

    if (args->canonical) {
        if (args->paths[0] == NULL)
            cat_canonical(stdin, "stdin");
        for (int i = 0; args->paths[i] != NULL; i++) {
            if (args->paths[i] == NULL)
                continue;
            if (args->paths[i][0] == '-') {
                cat_canonical(stdin, "stdin");
                continue;
            }
            cat_canonical(fopen(args->paths[i], "r"), args->paths[i]);
        }
    } else {
        if (args->paths[0] == NULL)
            cat(stdin, "stdin", args->end_of_line);
        for (int i = 0; args->paths[i] != NULL; i++) {
            if (args->paths[i] == NULL)
                continue;
            if (args->paths[i][0] == '-') {
                cat(stdin, "stdin", args->end_of_line);
                continue;
            }
            cat(fopen(args->paths[i], "r"), args->paths[i], args->end_of_line);
        }
    }

    free_args(args);
    return 0;
}
