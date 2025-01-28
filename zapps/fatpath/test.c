#include <stdio.h>
#include <stdlib.h>

char *profan_path_path(const char *exec);

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <path>\n", argv[0]);
        return 1;
    }

    char *path = profan_path_path(argv[1]);
    if (path == NULL) {
        perror("profan_path_path");
        return 1;
    }

    printf("%s\n", path);
    free(path);

    return 0;
}
