#include <stdio.h>

void set_val(int val);
int get_val(void);

int main(void) {
    set_val(10);
    printf("get_val() = %d\n", get_val());
    return 42;
}
