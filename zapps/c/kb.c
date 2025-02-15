/*****************************************************************************\
|   === kb.c : 2024 ===                                                       |
|                                                                             |
|    Command to change the keymap of the terminal                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int scancode_viewer(void) {
    int last_sc = 0;
    char min_val, max_val;

    printf("enter scancode, press ESC to exit\n");

    while ((last_sc = syscall_sc_get()) != 1) {
        if (last_sc == 0) {
            usleep(10000);
            continue;
        }

        min_val = profan_kb_get_char(last_sc, 0);
        max_val = profan_kb_get_char(last_sc, 1);

        printf("scancode: %d [%c %c]\n",
                last_sc,
                min_val ? min_val : '?',
                max_val ? max_val : '?'
        );
    }
    return 0;
}

int main(int argc, char *argv[]) {
    char *fullpath;

    if (argc == 2 && strcmp(argv[1], "-s") == 0) {
        return scancode_viewer();
    }

    if (argc != 2 || argv[1][0] == '-') {
        fputs("Usage: kb [-s | <mapname>]\n", stderr);
        return 1;
    }

    // assemble the path to the map
    fullpath = malloc(strlen(argv[1]) + 18);
    strcpy(fullpath, "/zada/keymap/");
    strcat(fullpath, argv[1]);
    strcat(fullpath, ".map");

    // change the keymap
    if (profan_kb_load_map(fullpath)) {
        fprintf(stderr, "kb: %s: Failed to load keymap\n", argv[1]);
        fputs("check that the map exists in /zada/keymap\n", stderr);

        free(fullpath);
        return 1;
    }

    free(fullpath);
    return 0;
}
