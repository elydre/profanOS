#include "addf.h"


int main(int addr, int arg) {
    INIT_AF(addr);

    AF_sys_run_binary();
    AF_fskprint();

    char path[] = "/bin/bombe.bin";

    fskprint("run: %d\n", arg);
    sys_run_binary(path, arg + 1);
    return 0;
}
