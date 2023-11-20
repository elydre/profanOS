#include <syscall.h>
#include <stdio.h>

int main(int argc, char **argv) {
    printf("\033[95mhave %d arguments\033[0m\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("\033[95m arg %d: \033[92m%s\033[0m\n", i, argv[i]);
    }
    return 0;
}
