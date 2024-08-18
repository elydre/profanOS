/*****************************************************************************\
|   === sc.c : 2024 ===                                                       |
|                                                                             |
|    Test file                                                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdio.h>

int main(void) {

    printf("before syscall\n");

    int ret = 0;

    asm volatile(
        "mov $0, %%eax\n"
        "mov $30, %%ebx\n"
        "mov $13, %%ecx\n"
        "int $0x80\n"
        // get return value
        "mov %%eax, %0\n"
        : "=r"(ret)
        :
        : "%eax", "%ebx", "%ecx"
    );

    printf("after syscall, ret = %d\n", ret);

    return 0;
}
