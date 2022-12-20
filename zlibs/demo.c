#include <syscall.h>
#include <iolib.h>

void init_lib();

int main() {
    init_lib();
    return 0;
}

void init_lib() {
    c_kprint("hello for lib init!\n");
}

void demo_func(int a) {
    fsprint("hello for lib demo!, a = %d\n", a);
}
