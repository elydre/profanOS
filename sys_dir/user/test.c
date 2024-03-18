#include <stdio.h>
void func1(void);

int main(void) {
    printf("Hello %p\n", func1);
    func1();
    return 0;
}
