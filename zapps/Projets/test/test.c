#include "test2.h"
#include <syscall.h>

int main() {
    Pair_t pair = (Pair_t) {1, 2};
    reverse(pair);
    c_fskprint("BITE !\n");
    return 0;
}