/*****************************************************************************\
|   === test.c : 2024 ===                                                     |
|                                                                             |
|    Test program                                                  .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan.h>
#include <stdio.h>

int aaaaa(void) {
    int ret, esp, ebp;

    asm volatile("movl %%esp, %0" : "=r" (esp));
    asm volatile("movl %%ebp, %0" : "=r" (ebp));

    printf("AAAAA!! esp = %x, ebp = %x\n", esp, ebp);

    asm volatile("int $0x80" : "=a" (ret) : "0" (30));

    asm volatile("movl %%esp, %0" : "=r" (esp));
    asm volatile("movl %%ebp, %0" : "=r" (ebp));

    printf("the fork of hades, ret = %d (esp = %x, ebp = %x)\n", ret, esp, ebp);
    return ret;
}

int test(void) {
    int ret;

    ret = aaaaa();

    printf("ret = %d, pid = %d\n", ret, syscall_process_pid());

    return ret;
}

int main(void) {
    int ret, esp;

    asm volatile("movl %%esp, %0" : "=r" (esp));
    printf("HELLO!! esp = %x\n", esp);

    ret = test();

    if (ret == 0) {
        printf("Test, child process, pid = %d\n", syscall_process_pid());
    } else {
        printf("Test, parent process, pid = %d\n", syscall_process_pid());
    }

    while (1);

    return 0;
}
