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

    asm volatile(
        "mov $0x4, %eax\n"
        "int $0x80\n"
    );

    printf("after syscall\n");

    return 0;
}
