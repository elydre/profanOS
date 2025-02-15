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
#include <string.h>

int compare_str(const void *a, const void *b) {
    return strcmp(*(const char **) a, *(const char **) b);
}

int main(int argc, char **argv, char **envp) {
    (void) argc;
    (void) argv;

    int env_size;
    for (env_size = 0; envp[env_size]; env_size++);

    qsort(envp, env_size, sizeof(char *), compare_str);

    for (int i = 0; envp[i]; i++) {
        printf("%s\n", envp[i]);
    }

    return 0;
}
