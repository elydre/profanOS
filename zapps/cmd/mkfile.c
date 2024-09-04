/*****************************************************************************\
|   === mkfile.c : 2024 ===                                                   |
|                                                                             |
|    Command to create a file - similar to touch                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <profan/filesys.h>
#include <profan.h>


int main(int argc, char **argv) {
    if (argc != 2 || !argv[1][0] || argv[1][0] == '-') {
        fprintf(stderr, "Usage: mkfile <file>\n");
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *full_path = assemble_path(pwd, argv[1]);

    int len = strlen(full_path);

    // get the parent path
    char *parent_path = malloc(256);

    // isolate the parent path
    for (int i = len - 1; i >= 0; i--) {
        if (full_path[i] == '/') {
            strncpy(parent_path, full_path, i);
            parent_path[i] = '\0';
            break;
        }
    }

    if (parent_path[0] == '\0') {
        strcpy(parent_path, "/");
    }

    // chek if the parent directory exists
    uint32_t parent_sid = fu_path_to_sid(ROOT_SID, parent_path);

    if (IS_SID_NULL(parent_sid) || !fu_is_dir(parent_sid)) {
        fprintf(stderr, "mkfile: %s: No such file or directory\n", parent_path);
    }

    // check if the file already exists
    else if (!IS_SID_NULL(fu_path_to_sid(ROOT_SID, full_path))) {
        fprintf(stderr, "mkfile: %s: Already exists\n", full_path);
    }

    // create the file
    else {
        fu_file_create(0, full_path);
        free(parent_path);
        free(full_path);
        return 0;
    }

    free(parent_path);
    free(full_path);

    return 1;
}
