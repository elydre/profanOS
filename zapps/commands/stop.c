#include <syscall.h>

int main(int argc, char **argv) {
    c_sys_shutdown();
    return 0;
}