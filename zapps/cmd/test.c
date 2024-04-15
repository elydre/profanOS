#include <profan/syscall.h>
#include <profan.h>
#include <stdio.h>

int global = 0;

int child(int param) {
    global = 1;
    return 0;
}

int main(void) {
    int pid = c_process_create(
        child,
        2,
        "oui",
        0
    );
    global = 42;
    c_process_wakeup(pid);

    profan_wait_pid(pid);
    printf("global: %d\n", global);

    return 0;
}
