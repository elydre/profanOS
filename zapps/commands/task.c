#include <syscall.h>

int main(int argc, char **argv) {
    c_task_debug_print();
    PATH_EXIT();
    return 0;
}
