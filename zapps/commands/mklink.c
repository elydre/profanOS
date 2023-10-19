#include <filesys.h>
#include <stdio.h>

int main(int argc, char **argv) {
    if (argc != 2) {
        printf("Usage: mklink <fullpath>\n");
        return 1;
    }

    fu_link_create(0, argv[1]);
    return 0;
}
