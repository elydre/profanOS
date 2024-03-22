#include <stdio.h>

int global = 8;

void func2(void) {
    printf("func2\n");
}

void func1(void) {
    printf("func1\n");
    func2();
}

void set_val(int val) {
    global = val;
}

int get_val(void) {
    return global;
}
