/*****************************************************************************\
|   === font.c : 2024 ===                                                     |
|                                                                             |
|    Command to change the font of the terminal                    .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/filesys.h>
#include <profan/panda.h>
#include <profan.h>

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    if (argc != 2 || argv[1][0] == '-') {
        fprintf(stderr, "Usage: font <file>\n");
        return 1;
    }

    char *full_path = assemble_path(pwd, argv[1]);

    uint32_t file = fu_path_to_sid(ROOT_SID, full_path);
    if (IS_SID_NULL(file) || !fu_is_file(file)) {
        fprintf(stderr, "font: %s: File not found\n", full_path);
        free(full_path);
        return 1;
    }

    if (panda_change_font(full_path)) {
        fprintf(stderr, "font: %s: Failed to change font\n", full_path);
        free(full_path);
        return 1;
    }

    free(full_path);
    return 0;
}
