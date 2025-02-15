/*****************************************************************************\
|   === starter.c : 2024 ===                                                  |
|                                                                             |
|    Really simple shell to run commands                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

char **rsplit(char **r, char *s, int *c) {
    int i = 0;
    while (*s == ' ') s++;
    while (s[i] && s[i] != ' ') i++;
    (r = realloc(r, (*c + 1) * sizeof(char *)))[*c] = i ? strndup(s, i) : 0;
    return i ? rsplit(r, s + i, ((*c)++, c)) : r;
}

int execute_line(char *line) {
    int argc = 0, res = 1;
    char **args;

    if (!(args = rsplit(NULL, line, &argc))[0]) {
        res = 0;
    } else if (strcmp(args[0], "cd") == 0) {
        if (chdir(args[1] ? args[1] : "/") == -1)
            fputs("cd: No such file or directory\n", stderr);
        else res = 0;
    } else if ((line = profan_path_path(args[0], 1))) {
        res = run_ifexist(line, argc, args);
        free(line);
    } else {
        fprintf(stderr, "%s: Command not found\n", args[0]);
    }

    for (int i = 0; args[i]; i++)
        free(args[i]);
    free(args);

    return res;
}

int main(void) {
    char  *line = NULL;
    size_t len = 0;

    for (int res = 0;; res = execute_line(line)) {
        printf("(%d) %s # ", res, profan_wd_path);
        fflush(stdout);
        if ((res = getline(&line, &len, stdin)) < 1)
            break;
        line[res - 1] = '\0';
    }

    putchar('\n');
    free(line);
    return 0;
}
