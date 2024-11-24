/*****************************************************************************\
|   === test.c : 2024 ===                                                     |
|                                                                             |
|    Tiny pipex implementation                                     .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <sys/wait.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>

char *dup_strft(char const *src, int start, int end) {
    char *res;
    int i;

    if (end < 0)
        end = strlen(src);

    res = malloc(sizeof(char) * (end - start + 1));

    for (i = 0; i < end - start && src[start + i]; i++)
        res[i] = src[start + i];
    res[i] = '\0';

    return res;
}

char **ft_split(char const *s, char c) {
    char **res;
    int start, i_tab = 0;

    res = malloc(sizeof(char *) * (strlen(s) + 1));

    for (int i = 0; s[i];) {
        while (s[i] && c == s[i])
            i++;
        start = i;
        while (s[i] && c != s[i])
            i++;
        if (start < i)
            res[i_tab++] = dup_strft(s, start, i);
    }
    res[i_tab] = NULL;
    return res;
}

void free_tab(char **tab) {
    for (int i = 0; tab[i]; i++)
        free(tab[i]);
    free(tab);
}

char *joinpath(char const *s1, char const *s2) {
    char *res = malloc(strlen(s1) + strlen(s2) + 2);
    strcpy(res, s1);
    strcat(res, "/");
    strcat(res, s2);
    return res;
}

char *get_path(char *file) {
    char **allpath, *path;

    if (file[0] == '/' || file[0] == '.')
        return strdup(file);

    allpath = ft_split(getenv("PATH"), ':');

    for (int i = 0; allpath[i]; i++) {
        path = joinpath(allpath[i], file);

        if (access(path, F_OK | X_OK) == 0)
            return free_tab(allpath), path;

        free(path);
    }

    free_tab(allpath);
    return (NULL);
}

void child(char *args, int fd1, int pipefd, char **envp) {
    char **cmd_argv;
    char *path;

    dup2(fd1, STDIN_FILENO);
    close(fd1);

    dup2(pipefd, STDOUT_FILENO);
    close(pipefd);

    cmd_argv = ft_split(args, ' ');

    if (cmd_argv == NULL) {
        fprintf(stderr, "pipex: failed to split command\n");
        return;
    }

    path = get_path(cmd_argv[0]);

    if (path == NULL) {
        fprintf(stderr, "pipex: command not found\n");
        free_tab(cmd_argv);
        return;
    }

    execve(path, cmd_argv, envp);

    fprintf(stderr, "pipex: failed to execute command\n");
    free_tab(cmd_argv);
}

int main(int argc, char **argv, char **envp) {
    pid_t pid1, pid2;
    int file1, file2;
    int pipefd[2];

    if (argc != 5) {
        printf("Usage: %s file1 cmd1 cmd2 file2\n", argv[0]);
        return 1;
    }

    file1 = open(argv[1], O_RDONLY);

    if (file1 == -1) {
        fprintf(stderr, "pipex: %s: %s\n", argv[1], strerror(errno));
        return 1;
    }

    file2 = open(argv[4], O_WRONLY | O_CREAT | O_TRUNC, 0644);

    if (file2 == -1) {
        fprintf(stderr, "pipex: %s: %s\n", argv[4], strerror(errno));
        close(file1);
        return 1;
    }

    if (pipe(pipefd) == -1) {
        fprintf(stderr, "pipex: %s\n", strerror(errno));
        close(file1);
        close(file2);
        return 1;
    }

    pid1 = fork();

    if (pid1 == -1) {
        fprintf(stderr, "pipex: %s\n", strerror(errno));
        close(pipefd[0]);
        close(pipefd[1]);
        close(file1);
        close(file2);
        return 1;
    }

    if (pid1 == 0) {
        child(argv[2], file1, pipefd[1], envp);
        return 1;
    }

    close(file1);
    close(pipefd[1]);

    pid2 = fork();

    if (pid2 == -1) {
        fprintf(stderr, "pipex: %s\n", strerror(errno));
        close(file2);
        close(pipefd[0]);
        return 1;
    }

    if (pid2 == 0) {
        child(argv[3], pipefd[0], file2, envp);
        return 1;
    }

    close(file2);
    close(pipefd[0]);

    waitpid(pid1, NULL, 0);
    waitpid(pid2, NULL, 0);

    return (0);
}
