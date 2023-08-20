#include <stdlib.h>
#include <stdio.h>

#include <filesys.h>
#include <syscall.h>

#define help_path "/zada/common/shell_help.txt"

int main(void) {
    sid_t file = fu_path_to_sid(ROOT_SID, help_path);

    if (!IS_NULL_SID(file) && fu_is_file(file)) {
        int size = fu_get_file_size(file);
        char *char_content = malloc(size + 2);

        fu_file_read(file, char_content, 0, size);

        c_ckprint(char_content, c_magenta);

        free(char_content);
        return 0;
    }
    printf("$3%s$B file not found\n", help_path);
    return 1;
}
