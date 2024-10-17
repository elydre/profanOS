#include <profan.h>
#include <stdlib.h>
#include <stdio.h>

void func2(void) {
    malloc(10);
}

void func1(void) {
    func2();
    malloc(42);
}

int main(void) {
    malloc(10);
    func1();
    return 0;
}
