#include <stdio.h>
void func1(void);
void set_val(int val);
int get_val(void);

int main(int argc, char *argv[]) {
    printf("argc: %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d]: %s\n", i, argv[i]);
    }

    printf("Hello %p\n", func1);
    func1();
    printf("Global: %d\n", get_val());
    set_val(5);
    printf("Global: %d\n", get_val());
    return 0;
}
