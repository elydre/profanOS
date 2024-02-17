#include <syscall.h>
#include <filesys.h>
#include <string.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>

void free_tab(char **tab) {
    for (int i = 0; tab[i]; i++)
        free(tab[i]);
    free(tab);
}

char *dup_strft(char *src, int start, int end) {
    char *res;
    int i;

    res = malloc(sizeof(char) * (end - start + 1));

    for (i = 0; i < end - start; i++)
        res[i] = src[start + i];
    res[i] = '\0';

    return res;
}

char **ft_split(char *s, char c) {
    int start, i_tab;
    char **res;

    start = 0;
    i_tab = 0;
    res = malloc(sizeof(char *) * (strlen(s) + 1));
    for (int i = 0; s[i]; i++) {
        if (s[i] != c) {
            start = i;
            while (s[i] && s[i] != c)
                i++;
            res[i_tab] = dup_strft(s, start, i);
            i_tab++;
        }
    }
    res[i_tab] = NULL;
    return res;
}

char *find_cmd(char *cmd) {
    char *path, *res;
    char **paths;
    sid_t sid;

    path = getenv("PATH");
    if (path == NULL)
        return NULL;
    paths = ft_split(path, ':');
    for (int i = 0; paths[i]; i++) {
        res = malloc(sizeof(char) * (strlen(paths[i]) + strlen(cmd) + 6));
        strcpy(res, paths[i]);
        strcat(res, "/");
        strcat(res, cmd);
        strcat(res, ".bin");
        sid = fu_path_to_sid(ROOT_SID, res);
        if (!IS_NULL_SID(sid) && fu_is_file(sid)) {
            free(paths);
            return res;
        }
        free(res);
    }
    free(paths);
    return NULL;
}

int internal_cd(int argc, char **argv) {
    char *current_path, *dir;
    sid_t sid;

    if (argc != 2) {
        printf("Usage: cd <dir>\n");
        return 1;
    }

    current_path = getenv("PWD");
    if (current_path == NULL)
        return 1;

    dir = assemble_path(current_path, argv[1]);
    fu_simplify_path(dir);

    sid = fu_path_to_sid(ROOT_SID, dir);
    if (IS_NULL_SID(sid) || !fu_is_dir(sid)) {
        printf("%s: directory not found\n", dir);
        free(dir);
        return 1;
    }

    setenv("PWD", dir, 1);
    free(dir);

    return 0;
}

int execute_line(char *line) {
    char **args, *cmd;
    int res, argc;

    args = ft_split(line, ' ');
    if (args[0] == NULL) {
        free(args);
        return 0;
    }
    for (argc = 0; args[argc]; argc++);
    if (strcmp(args[0], "cd") == 0) {
        res = internal_cd(argc, args);
        free_tab(args);
        return res;
    }

    cmd = find_cmd(args[0]);
    if (cmd == NULL) {
        printf("Command not found: %s\n", args[0]);
        return 1;
    }
    res = c_run_ifexist(cmd, argc, args);
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
        printf("(%d) %s # ", res, getenv("PWD"));
        fflush(stdout);
        line = open_input(NULL);
        if (line == NULL)
            break;
        remove_trailing_newline(line);
        res = execute_line(line);
        free(line);
    }
    return 0;
}
