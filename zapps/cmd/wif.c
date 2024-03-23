#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

char *read_stdin(int *len) {
    char *buffer = malloc(1025);
    int rcount = 0;

    if (isatty(0)) {
        puts("Enter content. Press ESC to finish.");
    }

    *len = 0;

    while ((rcount = fread(buffer + *len, 1, 1024, stdin)) > 0) {
        buffer = realloc(buffer, *len + rcount + 1025);
        *len += rcount;
    }

    buffer[*len] = '\0';

    return buffer;
}

int main(int argc, char** argv) {
    if (!(argc == 2 || argc == 3) || (argc == 2 && !strcmp(argv[1], "-h"))) {
        puts("Usage: wif <file> [content]");
        puts("If no content is given, stdin is used.");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = assemble_path(pwd, argv[1]);

    sid_t file = fu_path_to_sid(ROOT_SID, path);
    if (IS_NULL_SID(file) || !fu_is_file(file)) {
        file = fu_file_create(0, path);
        if (IS_NULL_SID(file)) {
            free(path);
            return 1;
        }
    }

    int len;
    if (argc == 2) {
        char *content = read_stdin(&len);
        fu_set_file_size(file, len);
        fu_file_write(file, content, 0, len);
        free(content);
    } else {
        len = strlen(argv[2]);
        fu_set_file_size(file, len);
        fu_file_write(file, argv[2], 0, len);
    }

    free(path);
    return 0;
}
