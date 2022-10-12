#include "syscall.h"
#include "test2.h"

int main(int arg) {
    print2();
    c_fskprint("BITE !\n");
    return arg;
}