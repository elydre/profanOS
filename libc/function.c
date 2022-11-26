#include <function.h>
#include <time.h>

int pow(int base, int exp) {
    int result = 1;
    while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

int abs(int x) {
    return x < 0 ? -x : x;
}

int rand() {
    static int *next = (int *) RAND_SAVE;
    *next = *next * 1103515245 + 12345;
    return (unsigned int) (* next / 65536) % 32768;
}

void init_rand() {
    static int *next = (int *) RAND_SAVE;
    *next = (uint32_t) time_get_boot();
}
