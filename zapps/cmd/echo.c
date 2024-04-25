/****** This file is part of profanOS **************************\
|   == echo.c ==                                     .pi0iq.    |
|                                                   d"  . `'b   |
|   Unix echo command implementation                q. /|\  u   |
|   print arguments to standard output               `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *app_ansi_color(char *str) {
    char *out = malloc(strlen(str) + 1);
    int dup_index, str_index = 0;

    for (dup_index = 0; str[str_index]; dup_index++) {
        if (str[str_index] == '\\' && (str[str_index + 1] == 'e' || str[str_index + 1] == 'E')) {
            out[dup_index] = '\e';
            str_index += 2;
        } else if (
            str[str_index] == '\\' &&
            str[str_index + 1] == '0' &&
            str[str_index + 2] == '3' &&
            str[str_index + 3] == '3'
        ) {
            out[dup_index] = '\e';
            str_index += 4;
        } else {
            out[dup_index] = str[str_index];
            str_index++;
        }
    }

    out[dup_index] = '\0';
    free(str);

    return out;
}

void show_help(void) {
    puts("Usage: echo [options] [string ...]\n"
        "Echo the STRING(s) to standard output.\n\n"
        "  -e     recognize ANSI color escape sequences\n"
        "  -h     display this help and exit\n"
        "  -n     do not output the trailing newline"
    );
}

char *read_stdin(void) {
    // read from stdin
    char *buffer = malloc(1024);
    int buffer_size = 0;
    int rcount = 0;

    while ((rcount = fread(buffer + buffer_size, 1, 1024, stdin)) > 0) {
        buffer = realloc(buffer, buffer_size + rcount + 1025);
        buffer_size += rcount;
    }

    buffer[buffer_size] = '\0';

    return buffer;
}

char *str_join(char *s1, char *s2) {
    char *output = malloc(strlen(s1) + strlen(s2) + 1);
    int i = 0;

    for (; s1[i]; i++) {
        output[i] = s1[i];
    }

    for (int j = 0; s2[j]; j++) {
        output[i + j] = s2[j];
    }

    output[i + strlen(s2)] = '\0';
    free(s1);

    return output;
}

char *get_text(char **args, int start) {
    if (!isatty(0)) {
        return read_stdin();
    }

    char *text = malloc(1);
    text[0] = '\0';

    for (int i = start; args[i]; i++) {
        text = str_join(text, args[i]);
        if (args[i + 1]) {
            text = str_join(text, " ");
        }
    }
    return text;
}

#define ECHO_ERRR 1
#define ECHO_ANSI 2
#define ECHO_HELP 4
#define ECHO_NONL 8

uint32_t parse_args(char **argv, int *offset) {
    uint32_t flags = 0;

    int i;
    for (i = 1; argv[i] && argv[i][0] == '-'; i++) {
        if (strcmp(argv[i], "-e") == 0) {
            flags |= ECHO_ANSI;
        } else if (strcmp(argv[i], "-h") == 0) {
            flags |= ECHO_HELP;
        } else if (strcmp(argv[i], "-n") == 0) {
            flags |= ECHO_NONL;
        } else {
            flags |= ECHO_ERRR;
        }
    }

    *offset = i;

    return flags;
}

int main(int argc, char **argv) {
    uint32_t flags;
    int offset;

    if (argv == 0) {
        printf("\n");
        return 0;
    }

    flags = parse_args(argv, &offset);

    if (flags & ECHO_ERRR) {
        fprintf(stderr, "echo: Invalid option -- '%s'\n", argv[offset]);
        fprintf(stderr, "Try 'echo -h' for more information.\n");
        return 1;
    }

    if (flags & ECHO_HELP) {
        show_help();
        return 0;
    }

    char *text = get_text(argv, offset);

    if (flags & ECHO_ANSI) {
        text = app_ansi_color(text);
    }

    fputs(text, stdout);

    if (!(flags & ECHO_NONL)) {
        fputs("\n", stdout);
    }

    free(text);

    return 0;
}
