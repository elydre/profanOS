#include <profan/syscall.h>
#include <stdio.h>

int main(void) {
    printf("Hello, World!\n");
    /*int ret = syscall_process_fork();

    printf("ret = %d, pid = %d\n", ret, syscall_process_pid());*/
    while (1) {
      //  printf("time: %d\n", syscall_timer_get_ms());
    }
    return 0;
}
