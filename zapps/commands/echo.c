#include <syscall.h>
#include <iolib.h>

int main(int argc, char** argv) {
    // TODO : add a way to keep the color between prints
    for (int i = 2; i < argc; i++) {
        fsprint("%s ", argv[i]);
    }
    fsprint("\n");
    return 0;
}
