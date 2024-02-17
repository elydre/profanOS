#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 1) {
        puts("Usage: rev\n");
        return 1;
    }

    char *line = NULL;
    int len = 0;
    int read_count = 1;

    while (1) {
        do {
            line = realloc(line, len + 1);
            read_count = read(0, line + len, 1);
            if (read_count == -1) {
                free(line);
                return 1;
            }
            len += read_count;
        } while (read_count > 0 && line[len - 1] != '\n');
        if (read_count == 0)
            break;
        for (int i = len - 1; i >= 0; i--) {
            if (line[i] == '\n')
                continue;
            putchar(line[i]);
        }
        putchar('\n');
        len = 0;
    }
    free(line);

    return 0;
}
