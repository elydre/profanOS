#include <function.h>

int pow(int base, int exp) {
    int result = 1;
    while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

void init_buffer(int buffer[], int size) {
    for (int i = 0; i < size; i++) buffer[i] = 0;
}
