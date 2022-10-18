#include <syscall.h>

int main(int argc, char **argv) {
    c_fskprint("Il y a %d arguments !\n", argc);
    c_fskprint("Les arguments sont : \n");
    for (int i = 0; i < argc; i++) {
        c_fskprint("Argument %d : %s\n", i, argv[i]);
    }
    return 0;
}