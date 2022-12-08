#include <syscall.h>
#include <iolib.h>

int main(int argc, char **argv) {
    fskprint("$4have %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        fskprint("$4arg %d: $1%s\n", i, argv[i]);
    }
    return 0;
}
