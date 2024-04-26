/*****************************************************************************\
|   === env.c : 2024 ===                                                      |
|                                                                             |
|    Unix command implementation - dump environment to stdout      .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv, char **envp) {
    for (int i = 0; envp[i]; i++) {
        printf("%s\n", envp[i]);
    }

    return 0;
}
