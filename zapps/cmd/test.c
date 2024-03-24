#include <stdio.h>

int print_info(void);
int double_num(int num);

int main(void) {
    print_info();
    int num = 5;
    printf("Doubled: %d\n", double_num(num));
    return 0;
}
