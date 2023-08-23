#include <filesys.h>
#include <stdio.h>

int main(int argc, char **argv) {
    fu_simplify_path(argv[1]);
    printf("%s\n", argv[1]);
    return 0;
}