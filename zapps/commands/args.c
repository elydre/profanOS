#include <syscall.h>
#include <stdio.h>

int main(int argc, char **argv) {
    printf("$4have %d arguments$$\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("$4 arg %d: $1%s$$\n", i, argv[i]);
    }
    return 0;
}
