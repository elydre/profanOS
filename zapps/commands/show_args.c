#include <syscall.h>

int main(int argc, char **argv) {
    c_fskprint("$4have %d arguments\n", argc);
    for (int i = 0; i < argc; i++) {
        c_fskprint("$4arg %d: $1%s\n", i, argv[i]);
    }
    return 0;
}
