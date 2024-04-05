#include <profan/syscall.h>

int main(int argc, char **argv) {
    if (argc == 1)
        c_sys_reboot();
    c_sys_shutdown();
    return 1;
}
