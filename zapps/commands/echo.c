#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

char *app_ansi_color(char *str) {
    char *out = malloc(strlen(str) + 1);
    int str_index = 0;

    for (int dup_index = 0; str[str_index]; dup_index++) {
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

    return out;
}

void show_help(void) {
    puts("Usage: echo [options] [string ...]\n"
        "Echo the STRING(s) to standard output.\n\n"
        "  -e        recognize ANSI color escape sequences\n"
        "  -h        display this help and exit"
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
    text = str_join(text, "\n");
    return text;
}

int main(int argc, char **argv) {
    int ansicolor = 0;
    int start = 1;

    if (argv == 0) {
        printf("\n");
        return 0;
    }

    if (argc > 1 && argv[1][0] == '-') {
        if (strcmp(argv[1], "-e") == 0) {
            ansicolor = 1;
        } else if (strcmp(argv[1], "-h") == 0) {
            show_help();
            return 0;
        } else {
            printf("echo: invalid option: %s\n", argv[1]);
            return 1;
        }
        start = 2;
    }

    char *text = get_text(argv, start);
    char *tmp = text;

    if (ansicolor) {
        tmp = app_ansi_color(text);
        free(text);
    }

    fputs(tmp, stdout);
    free(tmp);

    return 0;
}
