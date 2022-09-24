#include <function.h>
#include <time.h>

static uint32_t next;

int pow(int base, int exp) {
    int result = 1;
    while (exp) {
        if (exp & 1) result *= base;
        exp >>= 1;
        base *= base;
    }
    return result;
}

// rand function from osdev.org
int rand() {
    next = next * 1103515245 + 12345;
    return (int)(next / 65536) % 32768;
}

void init_rand() {
    next = (uint32_t) time_get_boot();
}
