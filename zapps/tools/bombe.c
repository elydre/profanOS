#include <syscall.h>
#include <iolib.h>


int main(int argc, char **argv) {

    char path[] = "/bin/tools/bombe.bin";

    fsprint("run: %d\n", argc);

    if (argc == 140) fsprint("done!\n");
    else c_run_ifexist(path, argc + 1, NULL);

    return 0;
}
