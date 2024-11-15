/*****************************************************************************\
|   === wif.c : 2024 ===                                                      |
|                                                                             |
|    Command to write data to a file                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

#define ERROR_MSG "Usage: wif <file> [content]\nIf no content is given, stdin is used.\n"

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
    if (argc < 2 || argc > 3) {
        fputs(ERROR_MSG, stderr);
        return 1;
    }

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            fprintf(stderr, "wif: %s: Invalid option\n" ERROR_MSG, argv[i]);
            return 1;
        }
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = profan_join_path(pwd, argv[1]);

    uint32_t file = fu_path_to_sid(SID_ROOT, path);
    if (IS_SID_NULL(file) || !fu_is_file(file)) {
        file = fu_file_create(0, path);
        if (IS_SID_NULL(file)) {
            free(path);
            return 1;
        }
    }

    int len;
    if (argc == 2) {
        char *content = read_stdin(&len);
        fu_file_set_size(file, len);
        fu_file_write(file, content, 0, len);
        free(content);
    } else {
        len = strlen(argv[2]);
        fu_file_set_size(file, len);
        fu_file_write(file, argv[2], 0, len);
    }

    free(path);
    return 0;
}
