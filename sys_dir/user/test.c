#include <stdio.h>
void func1(void);
void set_val(int val);
int get_val(void);

int main(void) {
    printf("Hello %p\n", func1);
    func1();
    printf("Global: %d\n", get_val());
    set_val(5);
    printf("Global: %d\n", get_val());
    return 0;
}
