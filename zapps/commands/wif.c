#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


int main(int argc, char** argv) {
    if (argc != 3) {
        puts("$BUsage: $3wif <file> <content>$7");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = malloc(strlen(pwd) + strlen(argv[1]) + 2);
    assemble_path(pwd, argv[1], path);

    sid_t file = fu_path_to_sid(ROOT_SID, path);

    if (!IS_NULL_SID(file) && fu_is_file(file)) {
        int len = strlen(argv[2]);

        fu_set_file_size(file, len);
        fu_file_write(file, (uint8_t *) argv[2], 0, len);

        free(path);
        return 0;
    }

    printf("$Bpath $3%s$B not found$7\n", path);
    free(path);
    return 1;
}
