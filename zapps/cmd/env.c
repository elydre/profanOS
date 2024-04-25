/****** This file is part of profanOS **************************\
|   == env.c ==                                      .pi0iq.    |
|                                                   d"  . `'b   |
|   Unix env command implementation                 q. /|\  u   |
|   prints environment variables to stdout           `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
    for (int i = 0; envp[i]; i++) {
        printf("%s\n", envp[i]);
    }

    return 0;
}
