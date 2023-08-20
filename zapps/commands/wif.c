#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


int main(int argc, char** argv) {
    if (argc != 4) {
        puts("$BUsage: $3wif <file> <content>\n");
        return 1;
    }

    char *path = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
    assemble_path(argv[1], argv[2], path);

    sid_t file = fu_path_to_sid(ROOT_SID, path);

    if (!IS_NULL_SID(file) && fu_is_file(file)) {
        int len = strlen(argv[3]);

        fu_set_file_size(file, len);
        fu_file_write(file, (uint8_t *) argv[3], 0, len);

        free(path);
        return 0;
    }

    printf("$Bpath $3%s$B not found\n", path);
    free(path);
    return 1;
}
