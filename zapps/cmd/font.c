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
    if (argc != 2 || argv[1][0] == '-') {
        fprintf(stderr, "Usage: font <file>\n");
        return 1;
    }

    uint32_t file = profan_resolve_path(argv[1]);
    if (IS_SID_NULL(file) || !fu_is_file(file)) {
        fprintf(stderr, "font: %s: File not found\n", argv[1]);
        return 1;
    }

    if (panda_change_font(file)) {
        fprintf(stderr, "font: %s: Failed to change font\n", argv[1]);
        return 1;
    }

    return 0;
}
