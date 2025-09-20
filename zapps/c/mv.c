/*****************************************************************************\
|   === mv.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - move file quickly               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/filesys.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <stdio.h>

char *dest_check_dir(char *dst, char *src) {
    // Check if the destination is a directory
    // If it is, append the source file name to it

    uint32_t sid = profan_path_resolve(dst);

    if (!fu_is_dir(sid))
        return strdup(dst);

    char *fullpath, *src_name;

    profan_path_sep(src, NULL, &src_name);
    fullpath = profan_path_join(dst, src_name);
    free(src_name);

    return fullpath;
}

int main(int argc, char **argv) {
    if (argc != 3 || argv[1][0] == '-' || argv[2][0] == '-') {
        fputs("Usage: mv <src> <dst>\n", stderr);
        return 1;
    }

    char *dest = dest_check_dir(argv[2], argv[1]);

    if (rename(argv[1], dest) < 0) {
        perror("mv: rename");
        free(dest);
        return 1;
    }

    free(dest);
    return 0;
}
