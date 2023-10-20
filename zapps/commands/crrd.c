#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("$BUsage: $3crrd <path>$$\n");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = malloc(strlen(pwd) + strlen(argv[1]) + 2);
    assemble_path(pwd, argv[1], path);

    sid_t file = fu_path_to_sid(ROOT_SID, path);

    if (!IS_NULL_SID(file)) {
        int size = c_fs_cnt_get_size(c_fs_get_main(), file);
        char *char_content = malloc(size + 2);

        c_fs_cnt_read(c_fs_get_main(), file, char_content, 0, size);
        profan_print_memory(char_content, size);

        free(char_content);
        free(path);
        return 0;
    }

    printf("$3%s$B file not found$$\n", path);
    free(path);
    return 1;
}
