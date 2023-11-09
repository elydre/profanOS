#include <stdlib.h>
#include <stdio.h>

#include <filesys.h>
#include <syscall.h>

#define help_path "/zada/common/shell_help.txt"

int main(int argc) {
    if (argc > 1) {
        puts("help: no arguments expected  -  use '<CDM> -h' or\n"
             "  '<CMD> --help' to get help on a specific command");
        return 1;
    }

    sid_t file = fu_path_to_sid(ROOT_SID, help_path);

    if (!IS_NULL_SID(file) && fu_is_file(file)) {
        int size = fu_get_file_size(file);
        char *char_content = malloc(size + 2);

        fu_file_read(file, char_content, 0, size);
        fputs(char_content, stdout);

        free(char_content);
        return 0;
    }
    printf("help: file %s not found\n", help_path);
    return 1;
}
