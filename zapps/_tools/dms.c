#include <stdio.h>

int *global_ptr;
int global_var;

char *aofa[] = {
    "aofa1",
    "aofa2",
};

int main(int argc, char **argv) {
    char array[256];
    int var;
    int *ptr;

    printf("var:          %p\n", &var);
    printf("ptr:          %p\n", &ptr);
    printf("array:        %p\n", &array);

    printf("main func:    %p\n", &main);
    printf("aofa[0][0]:   %p\n", &aofa[0][0]);
    printf("aofa:         %p\n", &aofa);

    printf("global_ptr:   %p\n", &global_ptr);
    printf("global_var:   %p\n", &global_var);

    return 0;
}
