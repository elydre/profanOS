#include "syscall.h"


int main(int arg) {

    char path[] = "/bin/tools/bombe.bin";

    c_fskprint("run: %d\n", arg);

    if (arg == 140) c_fskprint("done!\n");
    else c_sys_run_ifexist(path, 0, NULL);

    return arg;
}
