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

#include <profan/filesys.h>
#include <profan.h>

#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>

void free_tab(char **tab) {
    for (int i = 0; tab[i]; i++)
        free(tab[i]);
    free(tab);
}

char **ft_split(char *s, char c) {
    int i, j, k;
    char **res;

    res = calloc(strlen(s) + 1, sizeof(char *));
    for (i = j = k = 0; s[i]; i++) {
        if (s[i] == c) {
            res[j++] = strndup(s + k, i - k);
            k = i + 1;
        }
    }
    if (k != i)
        res[j++] = strndup(s + k, i - k);
    return res;
}

char *find_cmd(char *cmd) {
    char *path, *res;
    char **paths;
    uint32_t sid;

    path = getenv("PATH");
    if (path == NULL)
        return NULL;
    paths = ft_split(path, ':');
    for (int i = 0; paths[i]; i++) {
        res = malloc(strlen(paths[i]) + strlen(cmd) + 6);
        strcpy(res, paths[i]);
        strcat(res, "/");
        strcat(res, cmd);
        strcat(res, ".elf");
        sid = fu_path_to_sid(SID_ROOT, res);
        if (!IS_SID_NULL(sid) && fu_is_file(sid)) {
            free_tab(paths);
            return res;
        }
        free(res);
    }
    free_tab(paths);
    return NULL;
}

int internal_cd(int argc, char **argv) {
    if (argc != 2)
        fprintf(stderr, "Usage: cd <dir>\n");
    else if (chdir(argv[1]) == -1)
        fprintf(stderr, "cd: %s: No such file or directory\n", argv[1]);
    else
        return 0;
    return 1;
}

int execute_line(char *line) {
    char **args, *cmd;
    int res, argc;

    args = ft_split(line, ' ');

    if (args[0] == NULL)
        return (free(args), 0);

    for (argc = 0; args[argc]; argc++);

    if (strcmp(args[0], "cd") == 0) {
        res = internal_cd(argc, args);
        free_tab(args);
        return res;
    }

    cmd = find_cmd(args[0]);

    if (cmd == NULL) {
        fprintf(stderr, "%s: command not found\n", args[0]);
        free_tab(args);
        return 1;
    }

    res = run_ifexist(cmd, argc, args);
    free_tab(args);
    free(cmd);

    return res;
}

void remove_trailing_newline(char *s) {
    int len = strlen(s);
    if (len > 0 && s[len - 1] == '\n')
        s[len - 1] = '\0';
}

int main(void) {
    char *line;
    int res = 0;

    while (1) {
        printf("(%d) %s # ", res, profan_wd_path);
        fflush(stdout);
        line = profan_input(NULL);
        if (!line) {
            putchar('\n');
            profan_kfree(line);
            break;
        }
        remove_trailing_newline(line);
        res = execute_line(line);
        profan_kfree(line);
    }
    return 0;
}
