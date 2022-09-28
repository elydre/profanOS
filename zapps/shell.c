#include "addf.h"

int shell_command(int addr, char command[]);

#define SC_MAX 57
#define BFR_SIZE 68

static char current_dir[256] = "/";

// tu peut pas lancer une fonction qui s'appelle start? ça serait plus facile
int start(int addr, int arg) {
    INIT_AF(addr);
    AF_fskprint();
    AF_input_paste();
    AF_str_cpy();

    fskprint("Lancement de l'application shell...\n");

    char char_buffer[BFR_SIZE], last_buffer[BFR_SIZE];
    last_buffer[0] = '\0';
    while (1) {
        fskprint("profanOS [$9%s$7] -> ", current_dir);
        input_paste(char_buffer, BFR_SIZE, last_buffer, c_blue);
        str_cpy(last_buffer, char_buffer);
        fskprint("\n");
        if (shell_command(addr, char_buffer)) break;
        char_buffer[0] = '\0';
    }
    fskprint("Sortie de l'application shell...\n");
    return arg;
}

void assemble_path(int addr, char old[], char new[], char result[]) {
    INIT_AF(addr);
    AF_str_cpy();
    AF_str_len();
    AF_str_append();

    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}

void shell_help(int addr) {
    INIT_AF(addr);
    AF_fskprint();
    
    fskprint("$4alloc   - allocate *suffix* ko\n");
    fskprint("$4cat     - print file *suffix*\n");
    fskprint("$4cd      - change directory to *suffix*\n");
    fskprint("$4clear   - clear the screen\n");
    fskprint("$4echo    - print the arguments\n");
    fskprint("$4exit    - exit of the shell app\n");
    fskprint("$4free    - free *suffix* address\n");
    fskprint("$4go      - start *suffix* file as binary\n");
    fskprint("$4gpd     - get to the parent directory\n");
    fskprint("$4help    - show this help\n");
    fskprint("$4info    - show time, task & page info\n");
    fskprint("$4ls      - list the current directory\n");
    fskprint("$4mem     - show MLIST with colors\n");
    fskprint("$4mkdir   - create a new folder *suffix*\n");
    fskprint("$4mkfile  - create a new file *suffix*\n");
    fskprint("$4reboot  - reboot the system\n");
    fskprint("$4sc      - show the scancodes\n");
    fskprint("$4sleep   - sleep for a given time\n");
    fskprint("$4ss      - show int32 in the LBA *suffix*\n");
    fskprint("$4stop    - shutdown the system\n");
    fskprint("$4tree    - show the current directory tree\n");
    fskprint("$4udisk   - show used disk space\n");
    fskprint("$4usg     - show the usage of cpu\n");
    fskprint("$4yield   - yield to pid *suffix*\n");

}

void shell_ls(int addr) {
    INIT_AF(addr);
    AF_fs_get_folder_size();
    AF_malloc();
    AF_fs_type_sector();
    AF_fs_path_to_id();
    AF_fskprint();
    AF_fs_get_file_size();
    AF_fs_get_dir_content();
    AF_str_len();
    AF_free();

    int elm_count = fs_get_folder_size(current_dir);
    string_20_t *out_list = malloc(elm_count * sizeof(string_20_t));
    uint32_t *out_type = malloc(elm_count * sizeof(uint32_t));
    char tmp_path[256];
    fs_get_dir_content(fs_path_to_id(current_dir, 0), out_list, out_type);
    for (int i = 0; i < elm_count; i++) out_type[i] = fs_type_sector(out_type[i]);
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 3) {
            fskprint("$2%s", out_list[i].name);
            for (int j = 0; j < 22 - str_len(out_list[i].name); j++) fskprint(" ");
            assemble_path(addr, current_dir, out_list[i].name, tmp_path);
            fskprint("%d elm\n", fs_get_folder_size(tmp_path));
        }
    } for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 2) {
            fskprint("$1%s", out_list[i].name);
            for (int j = 0; j < 22 - str_len(out_list[i].name); j++) fskprint(" ");
            assemble_path(addr, current_dir, out_list[i].name, tmp_path);
            fskprint("%d sect\n", fs_get_file_size(tmp_path));
        }
    }
    free(out_list);
    free(out_type);
}

int shell_command(int addr, char command[]) {
    INIT_AF(addr);
    
    AF_str_start_split();
    AF_str_end_split();
    AF_clear_screen();
    AF_fskprint();
    AF_str_cpy();
    AF_str_len();
    AF_str_cmp();
    AF_malloc();
    AF_free();

    char *prefix = malloc(str_len(command) * sizeof(char)); // size of char is 1 octet
    char *suffix = malloc(str_len(command) * sizeof(char));

    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    // we test for exit first
    if (str_cmp(prefix, "exit") == 0) {
        free(prefix);
        free(suffix);
        return 1;
    } 

    else if (str_cmp(prefix, "clear") == 0)  clear_screen();
    else if (str_cmp(prefix, "echo") == 0)   fskprint("$4%s\n", suffix);
    else if (str_cmp(prefix, "help") == 0)   shell_help(addr);
    else if (str_cmp(prefix, "ls") == 0)     shell_ls(addr);

    free(prefix);
    free(suffix);
    return 0;
}