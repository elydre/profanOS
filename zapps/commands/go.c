#include <syscall.h>

void assemble_path(char old[], char new[], char result[]);

int main(int argc, char **argv) {
    char *path = c_malloc(256);
    c_fskprint("argv[0] = %s\n", argv[0]);
    c_fskprint("argv[1] = %s\n", argv[1]);
    c_fskprint("argv[2] = %s\n", argv[2]);
    char *suffix = c_malloc(256);
    c_str_cpy(suffix, argv[2]);
    char *current_dir = c_malloc(256);
    c_str_cpy(current_dir, argv[1]);
    if(!(c_str_count(suffix, '.'))) c_str_cat(suffix, ".bin");
    char *file = c_malloc(c_str_len(suffix)+c_str_len(current_dir)+2);
    assemble_path(current_dir, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        // when you go a file, it takes no arguments
        int argc = 1;
        char **argv = c_malloc(argc * sizeof(char *));
        // set argv[0] to the command name
        argv[0] = c_malloc(c_str_len(suffix) + 1);
        c_str_cpy(argv[0], file);
        c_sys_run_binary(file, 1, argc, argv);
        // free
        c_free(argv[0]);
        c_free(argv);
    }
    else c_fskprint("$3%s$B file not found\n", file);
    c_free(file);
    c_free(path);
    c_free(suffix);
    c_free(current_dir);
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {
    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}