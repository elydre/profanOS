#include <filesys.h>
#include <profan.h>
#include <panda.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    if (argc != 2) {
        printf("Usage: font <font path>\n");
        return 1;
    }

    char *full_path = assemble_path(pwd, argv[1]);

    sid_t file = fu_path_to_sid(ROOT_SID, full_path);
    if (IS_NULL_SID(file) || !fu_is_file(file)) {
        printf("File not found: %s\n", full_path);
        free(full_path);
        return 1;
    }

    if (panda_change_font(full_path)) {
        printf("Failed to change font, invalid file\n");
        free(full_path);
        return 1;
    }

    free(full_path);
    return 0;
}
