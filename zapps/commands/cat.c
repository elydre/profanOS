#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

void make_printable(char *str, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (str[i] == '\0')
            str[i] = 176;
    }
    if (str[size - 1] == '\n') {
        str[size] = '\0';
        return;
    }
    str[size] = '\n';
    str[size + 1] = '\0';
}

int main(int argc, char **argv) {
    if (argc < 3) {
        printf("$BUsage: $3cat <path>\n");
        return 1;
    }

    char *path = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
    assemble_path(argv[1], argv[2], path);

    sid_t file = fu_path_to_sid(ROOT_SID, path);

    if (!IS_NULL_SID(file) && fu_is_file(file)) {
        int size = fu_get_file_size(file);
        char *char_content = malloc(size + 2);

        fu_file_read(file, char_content, 0, size);
        make_printable(char_content, size);

        c_ckprint(char_content, c_magenta);

        free(char_content);
        free(path);
        return 0;
    }
    printf("$3%s$B file not found\n", path);
    free(path);
    return 1;
}
