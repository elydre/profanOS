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

// @LINK: libpf

#include <profan/carp.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>

int compare_str(const void *a, const void *b) {
    return strcmp(*(const char **) a, *(const char **) b);
}

void print_env(char **env) {
    int env_size;

    if (env == NULL || env[0] == NULL)
        return;

    for (env_size = 0; env[env_size]; env_size++);

    qsort(env, env_size, sizeof(char *), compare_str);

    for (int i = 0; env[i]; i++) {
        puts(env[i]);
    }
}

int main(int argc, char **argv) {
    carp_init("[-i] [NAME=VALUE] [cmd [arg] ...]", CARP_FNOMAX);

    carp_register('i', CARP_STANDARD, "start with an empty environment");

    if (carp_parse(argc, argv))
        return 1;

    if (carp_isset('i'))
        clearenv();

    const char **args = carp_get_files();
    int i;

    for (i = 0; args[i]; i++) {
        if (strchr(args[i], '=') != NULL) {
            putenv((char *) args[i]);
        } else {
            break;
        }
    }

    if (args[i] == NULL) {
        print_env(environ);
    } else {
        // execute the command
        execvp(args[i], (char * const *) &args[i]);
        perror("env: exec");
        return 127;
    }

    return 0;
}
