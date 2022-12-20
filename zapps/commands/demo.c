#include <stdlib.h>
#include <i_iolib.h>

int main(int argc, char **argv) {
    int *x = calloc(1, sizeof(int));
    fsprint("addr = %x\n", x);
    *x = 10;
    fsprint("value = %d\n", *x);
    free(x);
    int *y = malloc(sizeof(int));
    fsprint("addr = %x\n", y);
    *y = 10;
    fsprint("value = %d\n", *y);
    y = realloc(y, 2*sizeof(int));
    fsprint("addr = %x\n", y);
    *(y+1) = 20;
    fsprint("value = %d\n", *y);
    fsprint("value = %d\n", *(y+1));
    free(y);
    return 0;
}
