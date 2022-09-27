#include "addf.h"

int start(int addr, int arg) {
    INIT_AF(addr);
    AF_rainbow_print();

    rainbow_print("Hello world!\n");
    return arg;
}
