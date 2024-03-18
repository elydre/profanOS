#include <stdio.h>

void func2(void) {
    printf("func2\n");
}

void func1(void) {
    printf("func1\n");
    func2();
}
