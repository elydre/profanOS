#include <i_iolib.h>
#include <syscall.h>
#include <filesys.h>
#include <string.h>
#include <stdlib.h>
#include <profan.h>
#include <stdio.h>


#define BFR_SIZE 90
#define SC_MAX 57

#define SHELL_PROMPT "profanOS [$9%s$7] > "

static char current_dir[256] = "/";

void go(char *file, char *prefix, char *suffix);
int shell_command(char *command);

void start_split(char *s, char delim) {
    for (int i = 0; s[i] != '\0'; i++) {
        if (s[i] != delim) continue;
        s[i] = '\0';
        return;
    }
}

void end_split(char *s, char delim) {
    uint32_t len = strlen(s);
    int limit = 0;

    for (uint32_t i = 0; i < len; i++) {
        if (s[i] != delim) continue;
        limit = i + 1;
        break;
    }

    for (uint32_t i = limit; i < len; i++) {
        s[i - limit] = s[i];
    }

    s[len - limit] = '\0';
}

int str_count(char *str, char thing) {
    int total = 0;

    for (uint32_t i = 0; str[i] != '\0'; i++) {
        if (str[i] == thing) total++;
    }

    return total;
}

int main(int argc, char **argv) {
    char char_buffer[BFR_SIZE];

    while (1) {
        printf(SHELL_PROMPT, current_dir);
        fflush(stdout);
        open_input(char_buffer, BFR_SIZE);
        puts("\n");
        if (shell_command(char_buffer)) break;
    }
    return 0;
}

int shell_command(char *buffer) {
    char *prefix = malloc(strlen(buffer) + 5);
    char *suffix = malloc(strlen(buffer) + 5);
    strcpy(prefix, buffer);
    strcpy(suffix, buffer);
    start_split(prefix, ' ');
    end_split(suffix, ' ');

    if (strlen(buffer) == strlen(suffix)) {
        suffix[0] = '\0';
    }

    int return_value = 0;

    sid_t elm;

    // internal commands

    if (!strcmp(prefix, "exit")) {
        return_value = 1;
    } else if (!strcmp(prefix, "cd")) {
        char *new_path = calloc(256, sizeof(char));
        assemble_path(current_dir, suffix, new_path);
        elm = fu_path_to_sid(ROOT_SID, new_path);

        if (!IS_NULL_SID(elm) && fu_is_dir(elm))
            strcpy(current_dir, new_path);
        else
            printf("$3%s$B path not found\n", new_path);

        free(new_path);
    } else if (!strcmp(prefix, "go")) {
        if (!str_count(suffix, '.')) strncat(suffix, ".bin", 4);
        char *file = malloc(strlen(suffix) + strlen(current_dir) + 3);
        assemble_path(current_dir, suffix, file);
        suffix[0] = '\0';
        elm = fu_path_to_sid(ROOT_SID, file);

        if (!IS_NULL_SID(elm) && fu_is_file(elm))
            go(file, prefix, suffix);
        else
            printf("$3%s$B file not found\n", file);

        free(file);
    } else {  // shell command
        char *old_prefix = malloc(strlen(prefix) + 1);
        strcpy(old_prefix, prefix);
        if (!str_count(prefix, '.')) strncat(prefix, ".bin", 4);
        char *file = malloc(strlen(prefix) + 17);
        assemble_path("/bin/commands", prefix, file);
        elm = fu_path_to_sid(ROOT_SID, file);

        if (!IS_NULL_SID(elm) && fu_is_file(elm))
            go(file, old_prefix, suffix);
        else {
            assemble_path("/bin/fatpath", prefix, file);
            elm = fu_path_to_sid(ROOT_SID, file);
            if (!IS_NULL_SID(elm) && fu_is_file(elm)) {
                go(file, old_prefix, suffix);
            } else if (strcmp(old_prefix, "")) {
                printf("$3%s$B is not a valid command.\n", old_prefix);
            }
        }
        free(file);
        free(old_prefix);
    }

    free(prefix);
    free(suffix);
    return return_value;
}

void go(char *file, char *prefix, char *suffix) {
    int argc = str_count(suffix, ' ') + 2;
    if (suffix[0] != '\0') argc++;

    char **argv = malloc(argc * sizeof(char *));
    argv[0] = malloc(strlen(file) + 1);
    strcpy(argv[0], file);
    argv[1] = malloc(strlen(current_dir) + 1);
    strcpy(argv[1], current_dir);

    for (int i = 2; i < argc; i++) {
        argv[i] = malloc(strlen(suffix) + 1);
        strcpy(argv[i], suffix);
        start_split(argv[i], ' ');
        end_split(suffix, ' ');
    }

    c_run_ifexist(file, argc, argv);
    // free
    for (int i = 0; i < argc; i++) free(argv[i]);
    free(argv);
}
