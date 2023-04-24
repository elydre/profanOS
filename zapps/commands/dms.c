#include <stdio.h>

int *global_ptr;
int global_var;

char *test[] = {
    "test1",
    "test2",
};

int main(int argc, char **argv) {
    char text[256];
    int var;
    int *ptr;

    printf("global_var: %x\n", &global_var);
    printf("global_ptr: %x\n", &global_ptr);
    printf("var: %x\n", &var);
    printf("ptr: %x\n", &ptr);
    printf("text: %x\n", &text);
    printf("text[0]: %x\n", &text[0]);
    printf("test: %x\n", &test);
    printf("test[0]: %x\n", &test[0]);
    printf("test[0][0]: %x\n", &test[0][0]);
    printf("main: %x\n", &main);
    return 0;
}
