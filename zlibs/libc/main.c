#include <stdio.h>

void __attribute__((constructor)) my_constructor() {
    printf("Library loaded.\n");
}

void __attribute__((constructor)) my_constructor2() {
    printf("Library loaded 2.\n");
}
