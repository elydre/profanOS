#include <stdlib.h>
#include <profan.h>
#include <string.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    char *fullpath;

    if (argc != 2) {
        printf("Usage: kb <mapname>\n");
        return 1;
    }

    // assemble the path to the map
    fullpath = malloc(strlen(argv[1]) + 18);
    strcpy(fullpath, "/zada/keymap/");
    strcat(fullpath, argv[1]);
    strcat(fullpath, ".map");

    // change the keymap
    if (profan_kb_load_map(fullpath)) {
        printf("Failed to load keyboard map %s\n", argv[1]);
        printf("check that the map exists in /zada/keymap\n");

        free(fullpath);
        return 1;
    }

    free(fullpath);
    return 0;
}
