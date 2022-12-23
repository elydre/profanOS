#include <syscall.h>
#include <stdio.h>

int main(int argc, char** argv) {
    // TODO : add a way to keep the color between prints
    for (int i = 2; i < argc; i++) {
        printf("%s ", argv[i]);
    }
    printf("\n");
    return 0;
}
