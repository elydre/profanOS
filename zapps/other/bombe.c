#include "syscall.h"


int main(int arg) {

    char path[] = "/bin/bombe.bin";

    c_fskprint("run: %d\n", arg);

    if (arg == 140) c_fskprint("done!\n");
    else c_sys_run_binary(path, arg + 1);

    return arg;
}
