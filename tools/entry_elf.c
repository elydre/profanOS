// standard entry point for ELF compiled files

#include <stdlib.h>

extern int main(int argc, char **argv, char **envp);

int entry(int argc, char **argv, char **envp) {
    set_environ_ptr(envp);
    return main(argc, argv, envp);
}
