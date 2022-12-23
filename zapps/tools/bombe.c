#include <syscall.h>
#include <stdio.h>


int main(int argc, char **argv) {

    char path[] = "/bin/tools/bombe.bin";

    printf("run: %d\n", argc);

    if (argc == 140) printf("done!\n");
    else c_run_ifexist(path, argc + 1, NULL);

    return 0;
}
