#include <syscall.h>
#include <i_iolib.h>

int main(int argc, char **argv) {
    fsprint("$4have %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        fsprint("$4arg %d: $1%s\n", i, argv[i]);
    }
    return 0;
}
