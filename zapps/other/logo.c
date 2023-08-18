#include <syscall.h>
#include <filesys.h>
#include <i_iolib.h>
#include <stdlib.h>

#define LOGO_PATH "/zada/common/logo.itxt"

int main(int argc, char **argv) {
    sid_t file = fu_path_to_sid(ROOT_SID, LOGO_PATH);

    if (IS_NULL_SID(file) || !fu_is_file(file)) {
        color_print("logo file not found\n");
        return 1;
    }

    int size = fu_get_file_size(file);
    char *cnt = calloc(size + 1, sizeof(char));

    fu_read_file(file, cnt, size);

    color_print(cnt);
    free(cnt);

    return 0;
}
