#include <syscall.h>

int main(int argc, char** argv) {
    // TODO : add a way to keep the color between prints
    for (int i = 2; i < argc; i++) {
        c_fskprint("%s ", argv[i]);
    }
    c_fskprint("\n");
    return 0;
}