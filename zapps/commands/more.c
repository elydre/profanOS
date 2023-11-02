#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>
#include <i_time.h>
#include <panda.h>

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

int count_newlines(char *buffer) {
    int count = 0;
    for (int i = 0; buffer[i]; i++)
        if (buffer[i] == '\n')
            count++;
    return count;
}

int get_terminal_height(void) {
    uint32_t width, height;

    // check if the terminal is a panda terminal
    char *term = getenv("TERM");
    if (term == NULL || strcmp(term, "/dev/panda") != 0)
        return -1;
    panda_get_size(&width, &height);
    return height;
}

char get_user_input(void) {
    int scancode;
    char c;

    printf("\033[s$2 -- MORE - Press space to continue, q to quit -- $7");
    fflush(stdout);

    while (1) {
        scancode = c_kb_get_scfh();
        c = profan_kb_get_char(scancode, 0);
        if (c == 'q' || c == 'c' || scancode == 1) {
            c = 'q';
            break;
        }
        if (c == ' ' || scancode == 28) {
            c = ' ';
            break;
        }
        ms_sleep(100);
    }

    printf("\033[u\033[K");
    return c;
}

int main(int argc, char *argv[]) {
    if (argc != 1) {
        puts("more takes no arguments");
        puts("Usage: command | more");
        return 1;
    }

    if (isatty(0)) {
        puts("more takes input from stdin");
        puts("Usage: command | more");
        return 1;
    }

    char *buffer = read_stdin();

    if (buffer == NULL) {
        puts("Error reading stdin");
        return 1;
    }

    int line_count = 0;
    int line_limit = get_terminal_height() - 2;

    if (line_limit < 0) {
        for (int i = 0; buffer[i]; i++) {
            putchar(buffer[i]);
            if (buffer[i] == '\n' && get_user_input() == 'q')
                break;
        }
        free(buffer);
        return 0;
    }

    if (count_newlines(buffer) < line_limit) {
        printf(buffer);
        free(buffer);
        return 0;
    }

    // clear screen
    printf("\033[2J");
    fflush(stdout);

    for (int i = 0; buffer[i]; i++) {
        putchar(buffer[i]);
        if (buffer[i] != '\n')
            continue;

        line_count++;
        if (line_count < line_limit)
            continue;

        if (get_user_input() == 'q')
            break;

        line_limit += 8;
    }

    free(buffer);

    return 0;
}
