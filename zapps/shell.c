#include "syscall.h"

#define BFR_SIZE 90
#define HISTORY_SIZE 8

#define SC_MAX 57

static char current_dir[256] = "/";

int shell_command(char command[]);
void assemble_path(char old[], char new[], char result[]);

int main(int arg) {
    char char_buffer[BFR_SIZE];
    char **history = c_malloc(HISTORY_SIZE * sizeof(char*));
    for (int i = 0; i < HISTORY_SIZE; i++) {
        history[i] = c_malloc(BFR_SIZE * sizeof(char));
    }
    int current_history_size = 0;

    while (1) {
        c_fskprint("profanOS [$9%s$7] -> ", current_dir);
        c_input_wh(char_buffer, BFR_SIZE, c_blue, history, current_history_size);
        c_fskprint("\n");
        if (c_str_cmp(char_buffer, history[0]) && char_buffer[0] != '\0') {
            for (int i = HISTORY_SIZE - 1; i > 0; i--) c_str_cpy(history[i], history[i - 1]);
            if (current_history_size < HISTORY_SIZE) current_history_size++;
            c_str_cpy(history[0], char_buffer);
        }
        if (shell_command(char_buffer)) break;
    }
    for (int i = 0; i < HISTORY_SIZE; i++) c_free(history[i]);
    c_free(history);
    return arg;
}

int shell_command(char *buffer) {
    char *prefix = c_malloc(c_str_len(buffer)); // size of char is 1 octet
    char *suffix = c_malloc(c_str_len(buffer));

    int return_value = 0;

    c_str_cpy(prefix, buffer);
    c_str_cpy(suffix, buffer);
    c_str_start_split(prefix, ' ');
    c_str_end_split(suffix, ' ');

    // internal commands

    if (!c_str_cmp(prefix, "exit")) {
        return_value = 1;
    }
    
    // shell command
    else {
        if(!(c_str_count(prefix, '.'))) c_str_cat(prefix, ".bin");
        char *file = c_malloc(c_str_len(prefix)+c_str_len(current_dir)+2);
        assemble_path("/bin/commands", prefix, file);
        c_fskprint("Executing $9%s$7\n", file);
        if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2)
            c_sys_run_binary(file, 0, 0);
        else c_fskprint("$3%s$B file not found\n", file);
        c_free(file);
    }

    c_free(prefix);
    c_free(suffix);
    return return_value;
}

void assemble_path(char old[], char new[], char result[]) {

    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}