#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

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

void print_with_color(char *line, char *pattern) {
    char *pattern_start = strstr(line, pattern);
    char *pattern_end = pattern_start + strlen(pattern);
    char *line_end = strchr(line, '\0');

    fwrite(line, 1, pattern_start - line, stdout);
    printf("\e[95m");
    fwrite(pattern_start, 1, pattern_end - pattern_start, stdout);
    printf("\e[0m");
    fwrite(pattern_end, 1, line_end - pattern_end, stdout);
    printf("\n");
}

int main(int argc, char **argv) {
    if (argc != 2 || isatty(0)) {
        printf("Usage: <CMD> | grep <pattern>\n");
        return 1;
    }

    char *buffer = read_stdin();
    char *line = buffer;
    char *next_line = NULL;

    while (1) {
        next_line = strchr(line, '\n');
        if (next_line == NULL)
            break;
        *next_line = '\0';
        if (strstr(line, argv[1]) != NULL)
            print_with_color(line, argv[1]);
        line = next_line + 1;
    }

    free(buffer);
    return 0;
}
