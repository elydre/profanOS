#include <syscall.h>

void init_lib();

int main() {
    init_lib();
    return 0;
}

void init_lib() {
    c_kprint("hello for lib init!\n");
}
