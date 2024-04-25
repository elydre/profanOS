/****** This file is part of profanOS **************************\
|   == entry_elf.c ==                                .pi0iq.    |
|                                                   d"  . `'b   |
|   Standard entry point for ELF compiled files     q. /|\ .p   |
|                                                    '// \\'    |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <setjmp.h>

extern int main(int argc, char **argv, char **envp);

extern void __init_libc(char **envp, void *exit_func);
extern void __exit_libc();

void __entry_exit(int ret);

jmp_buf env;

int entry(int argc, char **argv, char **envp) {
    __init_libc(envp, __entry_exit);

    int val = setjmp(env);

    if (val == 0)
        val = main(argc, argv, envp) + 1;
    __exit_libc();

    return val - 1;
}

void __entry_exit(int ret) {
    longjmp(env, ret + 1);
}
