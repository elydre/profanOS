#include <stdio.h>

void __attribute__((constructor)) my_constructor() {
    fprintf(stderr, "Library loaded.\n");
}
