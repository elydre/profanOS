/*****************************************************************************\
|   === test.c : 2024 ===                                                     |
|                                                                             |
|    Temporary test file for malloc and fork functions             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <profan.h>
#include <assert.h>
#include <stdio.h>

int main() {
    void *addr = malloc(1024);
    printf("malloc(1024) = %p\n", addr);
    assert(addr != NULL);
    strcpy(addr, "Hello, world!");

    int pid = fork();

    if (pid == 0) {
        printf("child: %s\n", addr);
        memcpy(addr, "Goodbye, world!", 16);
        printf("child: %s\n", addr);
        free(addr);
        exit(0);
    } else {
        profan_wait_pid(pid);
    }

    printf("parent: %s\n", addr);
    free(addr);
    return 0;
}
