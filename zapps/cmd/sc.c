#include <stdio.h>

int main(void) {

    printf("before syscall\n");

    asm volatile(
        "mov $0x4, %eax\n"
        "int $0x80\n"
    );

    printf("after syscall\n");

    return 0;
}
