#include "syscall.h"

#define BFR_SIZE 90
#define HISTORY_SIZE 8

static char current_dir[256] = "/";

void parse_path(char path[], string_20_t liste_path[]);
int shell_command(char command[]);

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}

int shell_command(char command[]) {
    char *prefix = c_malloc(c_str_len(command)); // size of char is 1 octet
    char *suffix = c_malloc(c_str_len(command));

    int ret = 0;

    c_str_cpy(prefix, command);
    c_str_cpy(suffix, command);
    c_str_start_split(prefix, ' ');
    c_str_end_split(suffix, ' ');

    else if (c_str_cmp(prefix, "sleep") == 0)  c_ms_sleep(c_ascii_to_int(suffix) * 1000);
    else if (c_str_cmp(prefix, "yield") == 0)  c_yield((c_str_cmp(suffix, "yield") == 0) ? 1 : c_ascii_to_int(suffix));

    c_free(prefix);
    c_free(suffix);
    return ret;
}

void parse_path(char path[], string_20_t liste_path[]) {
    int index = 0;
    int index_in_str = 0;
    for (int i = 0; i < c_str_len(path); i++) {
        if (path[i] != '/') {
            liste_path[index].name[index_in_str] = path[i];
            index_in_str++;
        } else {
            liste_path[index].name[index_in_str] = '\0';
            index++;
            index_in_str = 0;
        }
    }
}
