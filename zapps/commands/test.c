#include <stdlib.h>

int main(void) {
    void *ptr = malloc(10);
    free(ptr);
    free(ptr);
    malloc(42);
    malloc(17);
    return 0;
}
