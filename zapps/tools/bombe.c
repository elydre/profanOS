#include "syscall.h"


int main(int argc, char **argv) {

    char path[] = "/bin/tools/bombe.bin";

    c_fskprint("run: %d\n", argc);

    if (argc == 140) c_fskprint("done!\n");
    else c_sys_run_binary(path, 0, argc + 1, NULL);

    return 0;
}
