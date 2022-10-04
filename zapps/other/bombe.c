#include "addf.h"


int main(int arg) {
    INIT_AF();

    AF_sys_run_binary();
    AF_fskprint();

    char path[] = "/bin/bombe.bin";

    fskprint("run: %d\n", arg);

    if (arg == 140) fskprint("done!\n");
    else sys_run_binary(path, arg + 1);

    return 0;
}
