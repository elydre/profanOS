#include <profan/filesys.h>
#include <profan/syscall.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

typedef struct {
    char  *full_path;
    char **args;
    int    arg_count;

    char  *input_file;
    char  *output_file;
    int    append_in_output;

    struct pipex_s *pipex_ptr;
} command_t;

typedef struct pipex_s {
    char  *prompt_env;
    char  *promt;

    command_t **commands;
    int         command_count;
} pipex_t;

typedef struct {
    char  *name;
    int  (*func)(char **);
} builtin_t;

#define CD_DEFAULT "/"
#define SHELL_NAME "lish"

/****************************************
 *                                     *
 *                UTILS                *
 *                                     *
****************************************/

char *readline(char *prompt) {
    int len;

    fputs(prompt, stdout);
    fflush(stdout);

    char *line = open_input(&len);
    if (len == 0) {
        puts("");
        free(line);
        return NULL;
    }
    if (line[len - 1] == '\n')
        line[len - 1] = '\0';
    return line;
}

char *dup_strft(char *src, int start, int end) {
    char *res;
    int i;

    if (end < 0)
        end = strlen(src);
    res = malloc(sizeof(char) * (end - start + 1));
    i = 0;
    while (i < end - start && src[start + i]) {
        res[i] = src[start + i];
        i++;
    }
    res[i] = '\0';
    return (res);
}

void free_tab(char **tab) {
    for (int i = 0; tab[i]; i++)
        free(tab[i]);
    free(tab);
}

int inchrset(char c, char *set) {
    for (int i = 0; set[i]; i++)
        if (c == set[i])
            return 1;
    return 0;
}

char *ft_strjoin(char *s1, char *s2) {
    int len1;
    int len2;
    char *res;
    int i;
    int j;

    len1 = strlen(s1);
    len2 = strlen(s2);
    res = malloc(sizeof(char) * (len1 + len2 + 1));
    if (res == NULL)
        return (NULL);
    i = 0;
    j = 0;
    while (i < len1) {
        res[i] = s1[i];
        i++;
    }
    while (j < len2) {
        res[i + j] = s2[j];
        j++;
    }
    res[i + j] = '\0';
    return (res);
}

char **ft_split(char *s, char c) {
    char **res;
    int start;
    int i_tab;
    int i;

    i = 0;
    start = 0;
    i_tab = 0;
    res = malloc(sizeof(char *) * (strlen(s) + 1));
    while (s[i]) {
        while (s[i] && c == s[i])
            i++;
        start = i;
        while (s[i] && c != s[i])
            i++;
        if (start < i)
            res[i_tab++] = dup_strft(s, start, i);
    }
    res[i_tab] = NULL;
    return (res);
}

void free_pipex(pipex_t *pipex) {
    for (int i = 0; i < pipex->command_count; i++) {
        if (pipex->commands[i] == NULL)
            continue;
        free(pipex->commands[i]->full_path);
        for (int j = 0; j < pipex->commands[i]->arg_count; j++)
            free(pipex->commands[i]->args[j]);
        free(pipex->commands[i]->args);
        free(pipex->commands[i]->input_file);
        free(pipex->commands[i]->output_file);
        free(pipex->commands[i]);
    }
    free(pipex->commands);
    free(pipex);
}

char **get_all_path(void) {
    char **res;
    char *tmp;

    tmp = getenv("PATH");
    if (tmp == NULL)
        return (NULL);
    res = ft_split(tmp, ':');
    return (res);
}

char *get_path(char *file) {
    char *path_part;
    char **allpath;
    char *path;
    sid_t sid;

    if (file[0] == '/' || file[0] == '.') {
        char *wd = getenv("PWD");
        if (!wd) return strdup(file);
        char *full_path = assemble_path(wd, file);
        return full_path;
    }

    allpath = get_all_path();
    if (allpath == NULL)
        return (strdup(file));
    for (int i = 0; allpath[i]; i++) {
        path_part = ft_strjoin(allpath[i], "/");
        path = ft_strjoin(path_part, file);
        free(path_part);
        sid = fu_path_to_sid(ROOT_SID, path);
        if (!IS_NULL_SID(sid) && fu_is_file(sid)) {
            free_tab(allpath);
            return path;
        }

        path_part = path;
        path = ft_strjoin(path_part, ".elf");
        free(path_part);
        sid = fu_path_to_sid(ROOT_SID, path);
        if (!IS_NULL_SID(sid) && fu_is_file(sid)) {
            free_tab(allpath);
            return path;
        }

        free(path);
    }
    free_tab(allpath);
    return (strdup(file));
}

void print_struct_pipex(pipex_t *pipex) {
    printf("command_count: %d\n", pipex->command_count);
    for (int i = 0; i < pipex->command_count; i++) {
        printf("command[%d]: %s\n", i, pipex->commands[i]->full_path);
        printf("| input_file: %s\n", pipex->commands[i]->input_file);
        printf("| output_file: %s%s\n", pipex->commands[i]->output_file,
                pipex->commands[i]->append_in_output ? " (append)" : "");
        printf("| arg_count: %d\n", pipex->commands[i]->arg_count);
        for (int j = 0; j <= pipex->commands[i]->arg_count; j++)
            printf("  | arg[%d]: %s\n", j, pipex->commands[i]->args[j]);
        printf("\n");
    }
}

/****************************************
 *                                     *
 *                SPLIT                *
 *                                     *
****************************************/

#define NOT_IN_QUOTE 0
#define IN_SIMPLE_QUOTE 1
#define IN_DOUBLE_QUOTE 2

char **ss_ft_split(char *str, char sep, char *other) {
    char **res;
    int i_res;
    int i_str;
    int start;
    int quote;

    quote = NOT_IN_QUOTE;
    i_res = 0;
    i_str = 0;
    res = malloc(sizeof(char *) * (strlen(str) + 1));
    while (str[i_str]) {
        while (str[i_str] == sep)
            i_str++;
        start = i_str;
        while (str[i_str] && (str[i_str] != sep || quote != NOT_IN_QUOTE)) {
            if (other && inchrset(str[i_str], other) && quote == NOT_IN_QUOTE) {
                if (start < i_str)
                    res[i_res++] = dup_strft(str, start, i_str);
                start = i_str;
                while (inchrset(str[i_str], other))
                    i_str++;
                break;
            }
            if (str[i_str] == '\'' && quote == NOT_IN_QUOTE)
                quote = IN_SIMPLE_QUOTE;
            else if (str[i_str] == '\'' && quote == IN_SIMPLE_QUOTE)
                quote = NOT_IN_QUOTE;
            else if (str[i_str] == '\"' && quote == NOT_IN_QUOTE)
                quote = IN_DOUBLE_QUOTE;
            else if (str[i_str] == '\"' && quote == IN_DOUBLE_QUOTE)
                quote = NOT_IN_QUOTE;
            i_str++;

        }
        if (start < i_str) {
            res[i_res++] = dup_strft(str, start, i_str);
        }
    }
    res[i_res] = NULL;
    return (res);
}

char *ss_remove_quotes(char *arg) {
    int quote = NOT_IN_QUOTE;
    int i_str = 0;

    for (int i = 0; arg[i]; i++) {
        if (arg[i] == '\'' && quote == NOT_IN_QUOTE)
            quote = IN_SIMPLE_QUOTE;
        else if (arg[i] == '\'' && quote == IN_SIMPLE_QUOTE)
            quote = NOT_IN_QUOTE;
        else if (arg[i] == '\"' && quote == NOT_IN_QUOTE)
            quote = IN_DOUBLE_QUOTE;
        else if (arg[i] == '\"' && quote == IN_DOUBLE_QUOTE)
            quote = NOT_IN_QUOTE;
        else {
            arg[i_str++] = arg[i];
        }
        if (arg[i + 1] == '\0')
            arg[i_str] = '\0';
    }
    return (arg);
}

char *ss_mv_single(char *arg, int last_exit) {
    int tmp, quote = NOT_IN_QUOTE;

    for (int j = 0; arg[j]; j++) {
        if (arg[j] == '\'' && quote == NOT_IN_QUOTE)
            quote = IN_SIMPLE_QUOTE;
        else if (arg[j] == '\'' && quote == IN_SIMPLE_QUOTE)
            quote = NOT_IN_QUOTE;
        else if (arg[j] == '\"' && quote == NOT_IN_QUOTE)
            quote = IN_DOUBLE_QUOTE;
        else if (arg[j] == '\"' && quote == IN_DOUBLE_QUOTE)
            quote = NOT_IN_QUOTE;
        else if (arg[j] == '$' && quote != IN_SIMPLE_QUOTE) {
            tmp = j++;
            if (arg[j] == '?')
                j++;
            else if (isdigit(arg[j]))
                j++;
            else while (arg[j] && (isalnum(arg[j]) || arg[j] == '_'))
                j++;
            if (j <= tmp + 1)
                continue;
            char *var = dup_strft(arg, tmp + 1, j);
            char *value;
            if (strcmp(var, "?") == 0) {
                value = malloc(12);
                itoa(last_exit, value, 10);
            } else {
                value = getenv(var);
            }
            if (!value) {
                value = "";
            }
            int value_len = strlen(value);
            arg = realloc(arg, strlen(arg) + value_len + 1);
            memmove(arg + tmp + value_len, arg + j, strlen(arg + j) + 1);
            for (int k = 0; k < value_len; k++)
                arg[tmp + k] = value[k];
            j = tmp + value_len - 1;
            value;
            free(var);
        }
    }
    return arg;
}

void ss_manage_variables(char **args, int last_exit) {
    for (int i = 0; args[i]; i++)
        args[i] = ss_mv_single(args[i], last_exit);
}

/****************************************
 *                                     *
 *               BULTIN                *
 *                                     *
****************************************/

int builtin_pwd(char **args) {
    if (args[1] != NULL) {
        fputs(SHELL_NAME": pwd: too many arguments\n", stderr);
        return 1;
    }
    char *pwd = getenv("PWD");
    if (pwd == NULL) {
        fputs(SHELL_NAME": pwd: PWD not set\n", stderr);
        return 1;
    }
    puts(pwd);
    return 0;
}

int builtin_export(char **args) {
    int ret = 0;
    for (int args_i = 1; args[args_i]; args_i++) {
        if (args[args_i][0] == '-') {
            fprintf(stderr, "export: %s: invalid option\n", args[args_i]);
            ret = 1;
            continue;
        }

        if (!isalpha(args[args_i][0]) && args[args_i][0] != '_') {
            fprintf(stderr, "export: %s: not a valid identifier\n", args[args_i]);
            ret = 1;
            continue;
        }

        for (int j = 1; args[args_i][j] && args[args_i][j] != '='; j++) {
            if (!isalnum(args[args_i][j]) && args[args_i][j] != '_') {
                fprintf(stderr, "export: %s: not a valid identifier\n", args[args_i]);
                ret = 1;
                continue;
            }
        }

        if (!strchr(args[args_i], '=')) {
            fprintf(stderr, "export: %s: not a valid identifier\n", args[args_i]);
            ret = 1;
            continue;
        }

        char *name = malloc(strlen(args[args_i]) + 1);
        char *value = malloc(strlen(args[args_i]) + 1);

        int i = -1;
        while (args[args_i][++i] != '=')
            name[i] = args[args_i][i];
        name[i] = '\0';

        int j;
        for (j = 0; args[args_i][i + j + 1] != '\0'; j++)
            value[j] = args[args_i][i + j + 1];
        value[j] = '\0';

        setenv(name, value, 1);
        free(value);
        free(name);
    }
    return ret;
}

int builtin_unset(char **args) {
    for (int i = 1; args[i] != NULL; i++) {
        unsetenv(args[i]);
    }
    return 0;
}

int builtin_cd(char **args) {
    // get argc

    if (args[1] && args[2]) {
        fputs(SHELL_NAME": cd: too many arguments\n", stderr);
        return 1;
    }

    char *current_path = getenv("PWD");
    if (current_path == NULL) {
        fputs(SHELL_NAME": cd: PWD not set\n", stderr);
        return 1;
    }

    // change to default if no arguments
    if (args[1] == NULL) {
        setenv("PWD", CD_DEFAULT, 1);
        return 0;
    }

    // assemble and simplify path
    char *dir = assemble_path(current_path, args[1]);
    fu_simplify_path(dir);

    sid_t dir_id = fu_path_to_sid(ROOT_SID, dir);
    if (IS_NULL_SID(dir_id) || !fu_is_dir(dir_id)) {
        fprintf(stderr, SHELL_NAME": cd: %s: no such file or directory\n", args[1]);
        free(dir);
        return 1;
    }

    // change directory
    setenv("PWD", dir, 1);
    free(dir);

    return 0;
}

builtin_t builtins[] = {
    {"pwd",    builtin_pwd},
    {"cd",     builtin_cd},
    {"export", builtin_export},
    {"unset",  builtin_unset},
    {NULL, NULL}
};

int (*get_builtin_func(char *cmd))(char **) {
    int i = -1;
    while (builtins[++i].name)
        if (strcmp(builtins[i].name, cmd) == 0)
            return (builtins[i].func);
    return (NULL);
}

/****************************************
 *                                     *
 *              UNCHILED               *
 *                                     *
****************************************/

int fucking_unchiled(pipex_t *pipex, int *ret) {
    if (pipex->command_count != 1)
        return 1;

    int (*builtin)(char **) = get_builtin_func(pipex->commands[0]->args[0]);
    if (!builtin)
        return 1;

    int in_fd, out_fd, saved_in, saved_out;
    saved_in = saved_out = -1;

    if (pipex->commands[0]->input_file) {
        in_fd = profan_open(pipex->commands[0]->input_file, O_RDONLY);
        saved_in = dup(STDIN_FILENO);
        if (dup2(in_fd, STDIN_FILENO) == -1) {
            fprintf(stderr, SHELL_NAME": %s: no such file or directory");
            *ret = 1;
            return 0;
        }
    }

    if (pipex->commands[0]->output_file) {
        out_fd = profan_open(pipex->commands[0]->output_file,
                O_WRONLY | O_CREAT | (pipex->commands[0]->append_in_output ? O_APPEND : O_TRUNC));
        saved_out = dup(STDOUT_FILENO);
        if (dup2(out_fd, STDOUT_FILENO) == -1) {
            fprintf(stderr, SHELL_NAME": %s: no such file or directory");
            dup2(saved_in, STDIN_FILENO);
            *ret = 1;
            return 0;
        }
    }

    if (!strcmp(pipex->commands[0]->args[0], "exit")) {
        fputs("exit\n", stderr);
        exit(pipex->commands[0]->args[1] ? atoi(pipex->commands[0]->args[1]) : 0);
    }

    *ret = builtin(pipex->commands[0]->args);

    dup2(saved_in, STDIN_FILENO);
    dup2(saved_out, STDOUT_FILENO);
    if (saved_in != -1) {
        close(saved_in);
        close(in_fd);
    }

    if (saved_out != -1) {
        close(saved_out);
        close(out_fd);
    }

    return 0;
}

void putendl_fd(char *s, int fd) {
    write(fd, s, strlen(s));
    write(fd, "\n", 1);
}

/****************************************
 *                                     *
 *               HEREDOC               *
 *                                     *
****************************************/

char *gen_heredoc_file(char *end) {
    static int i = 0;
    char *filename = malloc(30);
    strcpy(filename, "/tmp/sh_heredoc_");
    itoa(i++, filename + 16, 10);

    int fd = profan_open(filename, O_WRONLY | O_CREAT | O_TRUNC);
    if (fd == -1)
        return NULL;

    char *line = readline("> ");
    while (line) {
        if (strcmp(line, end) == 0) {
            free(line);
            break;
        }
        putendl_fd(line, fd);
        free(line);
        line = readline("> ");
    }
    close(fd);
    return filename;
}

/****************************************
 *                                     *
 *               PARSING               *
 *                                     *
****************************************/

// error codes:
// 0: no error
// 1: missing file name
// 2: empty file name
// 3: too many redirections
int manage_io(command_t *command, int last_exit) {
    command->input_file = NULL;
    command->output_file = NULL;
    command->append_in_output = 0;
    for (int j = 0; j < command->arg_count; j++) {
        if (command->args[j] == NULL)
            continue;

        if (strcmp(command->args[j], "<") == 0) {
            if (command->input_file != NULL)
                free(command->input_file);
            if (command->args[j + 1] == NULL)
                return 1;
            command->args[j + 1] = ss_mv_single(command->args[j + 1], last_exit);
            command->input_file = ss_remove_quotes(command->args[j + 1]);
            if (!command->input_file[0])
                return 2;
            if (inchrset(command->input_file[0], "<>"))
                return 3;
            free(command->args[j]);
            command->args[j + 1] = NULL;
            command->args[j] = NULL;
            continue;
        }

        if (strcmp(command->args[j], ">") == 0) {
            if (command->output_file != NULL)
                free(command->output_file);
            if (command->args[j + 1] == NULL)
                return 1;
            command->args[j + 1] = ss_mv_single(command->args[j + 1], last_exit);
            command->output_file = ss_remove_quotes(command->args[j + 1]);
            if (!command->output_file[0])
                return 2;
            if (inchrset(command->output_file[0], "<>"))
                return 3;
            free(command->args[j]);
            command->args[j + 1] = NULL;
            command->args[j] = NULL;
            continue;
        }

        if (strcmp(command->args[j], ">>") == 0) {
            if (command->output_file != NULL)
                free(command->output_file);
            if (command->args[j + 1] == NULL)
                return 1;
            command->args[j + 1] = ss_mv_single(command->args[j + 1], last_exit);
            command->output_file = ss_remove_quotes(command->args[j + 1]);
            if (!command->output_file[0])
                return 2;
            if (inchrset(command->output_file[0], "<>"))
                return 3;
            free(command->args[j]);
            command->append_in_output = 1;
            command->args[j + 1] = NULL;
            command->args[j] = NULL;
            continue;
        }

        if (strcmp(command->args[j], "<<") == 0) {
            if (command->input_file != NULL)
                free(command->input_file);
            if (command->args[j + 1] == NULL)
                return 1;
            command->args[j + 1] = ss_mv_single(command->args[j + 1], last_exit);
            ss_remove_quotes(command->args[j + 1]);
            if (inchrset(command->args[j + 1][0], "<>"))
                return 3;
            command->input_file = gen_heredoc_file(command->args[j + 1]);
            free(command->args[j + 1]);
            free(command->args[j]);
            command->args[j + 1] = NULL;
            command->args[j] = NULL;
            continue;
        }

        // <<<, >>>>>, etc raise an error
        if (command->args[j][0] == '<' || command->args[j][0] == '>') {
            return 3;
        }
    }
    return 0;
}

void raise_manage_io(int code, int command_nb) {
    if (code == 1)
        fprintf(stderr, SHELL_NAME": missing file name for redirection in command %d\n", command_nb);
    else if (code == 2)
        fprintf(stderr, SHELL_NAME": empty file name for redirection in command %d\n", command_nb);
    else if (code == 3)
        fprintf(stderr, SHELL_NAME": invalid redirection syntax in command %d\n", command_nb);
}

int start_end_pipe(char *str) {
    for (int i = 0; str[i]; i++) {
        if (str[i] == ' ' || str[i] == '\t')
            continue;
        if (str[i] == '|') {
            fprintf(stderr, SHELL_NAME": pipe at start of command\n");
            return 1;
        }
        else
            break;
    }
    for (int i = strlen(str) - 1; i >= 0; i--) {
        if (str[i] == ' ' || str[i] == '\t')
            continue;
        if (str[i] == '|') {
            fprintf(stderr, SHELL_NAME": pipe at end of command\n");
            return 1;
        }
        else
            break;
    }
    return 0;
}

pipex_t *parse_to_pipex(char *str, int last_exit) {
    int tmp;
    if (start_end_pipe(str) == 1) {
        return NULL;
    }

    pipex_t *pipex = malloc(sizeof(pipex_t));
    pipex->command_count = 0;

    char **fspt = ss_ft_split(str, '|', NULL);
    for (int i = 0; fspt[i]; i++)
        pipex->command_count++;

    pipex->commands = malloc(sizeof(command_t *) * pipex->command_count + 1);
    for (int i = 0; i < pipex->command_count; i++)
        pipex->commands[i] = NULL;
    for (int i = 0; fspt[i]; i++) {
        command_t *command = malloc(sizeof(command_t));
        command->full_path = NULL;
        command->pipex_ptr = pipex;
        command->args = ss_ft_split(fspt[i], ' ', "<>");

        command->arg_count = 0;
        for (int j = 0; command->args[j]; j++)
            command->arg_count++;

        if ((tmp = manage_io(command, last_exit))) {
            raise_manage_io(tmp, i + 1);
            free_pipex(pipex);
            return NULL;
        }

        ss_manage_variables(command->args, last_exit);
        for (int j = 0; command->args[j]; j++)
            ss_remove_quotes(command->args[j]);

        pipex->commands[i] = command;
        free(fspt[i]);
    }
    free(fspt);
    return pipex;
}

/****************************************
 *                                     *
 *                PIPEX                *
 *                                     *
****************************************/

void close_fds(pipex_t *pipex, int *fds, int i) {
    if (i != 0 || pipex->commands[i]->input_file != NULL)
        close(fds[i * 2]);
    if (i != pipex->command_count - 1 || pipex->commands[i]->output_file != NULL)
        close(fds[i * 2 + 1]);
}

int start_pipex(pipex_t *pipex) {
    int *fds = malloc(pipex->command_count * 2 * sizeof(int));
    int *pids = calloc(pipex->command_count, sizeof(int));
    int pipefd[2];

    for (int i = 0; i < pipex->command_count; i++) {
        if (i != 0 || pipex->commands[i]->input_file != NULL) {
            if (pipex->commands[i]->input_file == NULL) {
                // already in pipe
                fds[i * 2] = pipefd[0];
            } else {
                fds[i * 2] = profan_open(pipex->commands[i]->input_file, O_RDONLY);
            }
        }
        if (i != pipex->command_count - 1 || pipex->commands[i]->output_file != NULL) {
            if (pipex->commands[i]->output_file == NULL) {
                pipe(pipefd);
                fds[i * 2 + 1] = pipefd[1];
            } else {
                fds[i * 2 + 1] = profan_open(pipex->commands[i]->output_file,
                        O_WRONLY | O_CREAT | (pipex->commands[i]->append_in_output ? O_APPEND : O_TRUNC));
            }
        }
    }

    for (int i = 0; i < pipex->command_count; i++) {
        sid_t sid = fu_path_to_sid(ROOT_SID, pipex->commands[i]->full_path);
        if (IS_NULL_SID(sid) || !fu_is_file(sid)) {
            fprintf(stderr, SHELL_NAME": %s: command not found\n", pipex->commands[i]->args[0]);
            close_fds(pipex, fds, i);
            continue;
        }

        if (run_ifexist_full((runtime_args_t) {
                pipex->commands[i]->full_path,
                pipex->commands[i]->arg_count,
                pipex->commands[i]->args,
                __get_environ_ptr(),
                2
            }, pids + i) == -1
        ) {
            close_fds(pipex, fds, i);
            continue;
        }

        if (i != 0 || pipex->commands[i]->input_file != NULL) {
            if (dup2(fds[i * 2], fm_resol012(0, pids[i])) == -1) {
                fprintf(stderr, SHELL_NAME": %s: no such file or directory\n", pipex->commands[i]->input_file);
                close_fds(pipex, fds, i);
                c_exit_pid(pids[i], 1, 0);
                continue;
            }
        }

        if (i != pipex->command_count - 1 || pipex->commands[i]->output_file != NULL) {
            if (dup2(fds[i * 2 + 1], fm_resol012(1, pids[i])) == -1) {
                fprintf(stderr, SHELL_NAME": %s: no such file or directory\n", pipex->commands[i]->output_file);
                close_fds(pipex, fds, i);
                c_exit_pid(pids[i], 1, 0);
                continue;
            }
        }

        close_fds(pipex, fds, i);
        c_process_wakeup(pids[i]);
    };

    free(fds);

    int exit_code = 0;
    for (int i = 0; i < pipex->command_count; i++) {
        if (pids[i] > 0)
            exit_code = profan_wait_pid(pids[i]);
    }

    free(pids);

    return exit_code;
}

/****************************************
 *                                     *
 *                MAIN                 *
 *                                     *
****************************************/

void remove_null_args(pipex_t *pipex) {
    for (int i = 0; i < pipex->command_count; i++) {
        int j = 0;
        while (j < pipex->commands[i]->arg_count) {
            if (pipex->commands[i]->args[j] == NULL || strcmp(pipex->commands[i]->args[j], "") == 0) {
                if (pipex->commands[i]->args[j])
                    free(pipex->commands[i]->args[j]);
                for (int k = j; k < pipex->commands[i]->arg_count - 1; k++)
                    pipex->commands[i]->args[k] = pipex->commands[i]->args[k + 1];
                pipex->commands[i]->arg_count--;
            }
            else
                j++;
        }
    }
    for (int i = 0; i < pipex->command_count; i++)
        pipex->commands[i]->args[pipex->commands[i]->arg_count] = NULL;
}

void patch_tilde(pipex_t *pipex, char *home) {
    if (home == NULL)
        return;
    for (int i = 0; i < pipex->command_count; i++) {
        if (pipex->commands[i]->input_file && pipex->commands[i]->input_file[0] == '~') {
            char *tmp = ft_strjoin(home, pipex->commands[i]->input_file + 1);
            free(pipex->commands[i]->input_file);
            pipex->commands[i]->input_file = tmp;
        }
        if (pipex->commands[i]->output_file && pipex->commands[i]->output_file[0] == '~') {
            char *tmp = ft_strjoin(home, pipex->commands[i]->output_file + 1);
            free(pipex->commands[i]->output_file);
            pipex->commands[i]->output_file = tmp;
        }
        for (int j = 0; j < pipex->commands[i]->arg_count; j++) {
            if (pipex->commands[i]->args[j][0] == '~') {
                char *tmp = ft_strjoin(home, pipex->commands[i]->args[j] + 1);
                free(pipex->commands[i]->args[j]);
                pipex->commands[i]->args[j] = tmp;
            }
        }
    }
}

int just_spaces(char *str) {
    for (int i = 0; str[i]; i++)
        if (str[i] != ' ' && str[i] != '\t')
            return 0;
    return 1;
}

char *get_prompt(int last_exit) {
    char *pwd = getenv("PWD");
    if (pwd == NULL) pwd = "<?>";

    char *prompt = malloc(strlen(pwd) + 50 + strlen(SHELL_NAME));
    if (last_exit)
        sprintf(prompt, "\e[37m(\e[91m#%3d\e[37m)-[\e[36m%s\e[37m] $ \e[0m", last_exit, pwd);
    else
        sprintf(prompt, "\e[37m("SHELL_NAME")-[\e[36m%s\e[37m] $ \e[0m", pwd);

    return prompt;
}

void sh_print_help(int full) {
    puts("Usage: "SHELL_NAME" [-h|-d|-c command]");
    if (!full) {
        puts("try '"SHELL_NAME" -h' for more information");
        return;
    }
    puts(
        "Options:\n"
        "  -c  run command and exit\n"
        "  -h  display this help and exit\n"
        "  -d  debug mode\n"
        "Builtins:\n"
        "  pwd    print working directory\n"
        "  cd     change directory\n"
        "  exit   exit the shell\n"
        "  export set environment variables\n"
        "  unset  unset environment variables\n"
        "Redirections:\n"
        "  >     output to file\n"
        "  >>    append output to file\n"
        "  <     input from file\n"
        "  <<    input from heredoc\n"
        "  |     pipe"
    );
}

int sh_parse_args(int argc, char **argv) {
    if (argc < 2)
        return 0;
    if (argc == 2 && strcmp(argv[1], "-d") == 0)
        return 1;
    if (argc == 2 && strcmp(argv[1], "-h") == 0)
        return 2;
    if (argc == 3 && strcmp(argv[1], "-c") == 0)
        return 3;
    return -1;
}

int main(int argc, char **argv) {
    char *line, *prompt;
    int i, last_exit = 0;
    pipex_t *pipex;

    int mode = sh_parse_args(argc, argv);

    if (mode == -1) {   // invalid args
        sh_print_help(0);
        return 1;
    }

    if (mode == 2) {  // help
        sh_print_help(1);
        return 0;
    }

    setenv("PWD", "/", 0);

   do {
        if (mode == 3) {
            line = strdup(argv[2]);
        } else {
            prompt = get_prompt(last_exit);
            line = readline(prompt);
            free(prompt);
        }

        if (line == NULL || strcmp(line, "exit") == 0) {
            fputs("exit\n", stderr);
            free(line);
            break;
        }

        if (just_spaces(line)) {
            free(line);
            continue;
        }

        for (i = 0; line[i]; i++)
            if (line[i] == '\t')
                line[i] = ' ';

        pipex = parse_to_pipex(line, last_exit);
        free(line);

        if (!pipex) {
            last_exit = 1;
            continue;
        }

        if (pipex->command_count == 0) {
            free_pipex(pipex);
            last_exit = 0;
            continue;
        }

        remove_null_args(pipex);
        patch_tilde(pipex, getenv("HOME"));
        for (i = 0; i < pipex->command_count; i++) {
            if (pipex->commands[i]->arg_count == 0) {
                fprintf(stderr, SHELL_NAME": no executable given in command %d\n", i + 1);
                last_exit = 1;
                i = -1;
                break;
            }
            pipex->commands[i]->full_path = get_path(pipex->commands[i]->args[0]);
        }
        if (i == -1) {
            free_pipex(pipex);
            continue;
        }

        for (i = 1; i < pipex->command_count; i++)
            if (pipex->commands[i]->input_file == NULL && pipex->commands[i - 1]->output_file != NULL)
                pipex->commands[i]->input_file = strdup("/dev/null");

        for (i = 0; i < pipex->command_count - 1; i++)
            if (pipex->commands[i]->output_file == NULL && pipex->commands[i + 1]->input_file != NULL)
                pipex->commands[i]->output_file = strdup("/dev/null");

        if (mode == 1)
            print_struct_pipex(pipex);

        if (fucking_unchiled(pipex, &last_exit))
            last_exit = start_pipex(pipex);

        free_pipex(pipex);
    } while (mode != 3);

    return 0;
}
