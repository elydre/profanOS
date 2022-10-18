#include "syscall.h"

#define BFR_SIZE 90
#define HISTORY_SIZE 8

#define SC_MAX 57

static char current_dir[256] = "/";

void parse_path(char path[], string_20_t liste_path[]);
int shell_command(char command[]);
void gpd();

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
    return 0;
}

void assemble_path(char old[], char new[], char result[]) {

    result[0] = '\0'; c_str_cpy(result, old);
    if (result[c_str_len(result) - 1] != '/') c_str_append(result, '/');
    for (int i = 0; i < c_str_len(new); i++) c_str_append(result, new[i]);
}

void shell_ls(char path[]) {
    char ls_path[256];
    if (path[0] == '\0') c_str_cpy(ls_path, current_dir);
    else assemble_path(current_dir, path, ls_path);
    
    int elm_count = c_fs_get_folder_size(ls_path);
    string_20_t *out_list = c_malloc(elm_count * sizeof(string_20_t));
    uint32_t *out_type = c_malloc(elm_count * sizeof(uint32_t));
    char tmp_path[256];
    c_fs_get_dir_content(c_fs_path_to_id(ls_path, 0), out_list, out_type);
    for (int i = 0; i < elm_count; i++) out_type[i] = c_fs_type_sector(out_type[i]);
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 3) {
            c_fskprint("$2%s", out_list[i].name);
            for (int j = 0; j < 22 - c_str_len(out_list[i].name); j++) c_fskprint(" ");
            assemble_path(ls_path, out_list[i].name, tmp_path);
            c_fskprint("%d elm\n", c_fs_get_folder_size(tmp_path));
        }
    } for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 2) {
            c_fskprint("$1%s", out_list[i].name);
            for (int j = 0; j < 22 - c_str_len(out_list[i].name); j++) c_fskprint(" ");
            assemble_path(ls_path, out_list[i].name, tmp_path);
            c_fskprint("%d sect\n", c_fs_get_file_size(tmp_path));
        }
    }
    c_free(out_list);
    c_free(out_type);
}

void print_scancodes() {
    c_clear_screen();
    c_rainbow_print("enter scancode press ESC to exit\n");
    int last_sc = 0;
    while (last_sc != 1) {
        while (last_sc == c_kb_get_scancode());
        last_sc = c_kb_get_scancode();
        c_fskprint("$4\nscancode:   $1%d", last_sc);

        if (last_sc > SC_MAX) {
            c_fskprint("\n$5unknown scancode\n");
            continue;
        }

        c_fskprint("$4\nletter min: $1%c", c_kb_scancode_to_char(last_sc, 0));
        c_fskprint("$4\nletter maj: $1%c\n", c_kb_scancode_to_char(last_sc, 1));
    }
    c_clear_screen();
}

void show_disk_LBA(char suffix[]) {
    int LBA = 0;
    if (c_str_cmp(suffix, "ss") != 0) LBA = c_ascii_to_int(suffix);
    uint32_t outbytes[128];
    char tmp[10];
    c_ata_read_sector((uint32_t) LBA, outbytes);
    for (int i = 0; i < 128; i++) {
        c_int_to_ascii(outbytes[i], tmp);
        c_ckprint(tmp, c_magenta);
        for (int j = c_str_len(tmp); j < 10; j++) c_kprint(" ");
    }
}

void shell_tree(char path[], int rec) {
    int elm_count = c_fs_get_folder_size(path);
    string_20_t *out_list = c_malloc(elm_count * sizeof(string_20_t));
    uint32_t *out_type = c_malloc(elm_count * sizeof(uint32_t));
    char tmp_path[256];
    c_fs_get_dir_content(c_fs_path_to_id(path, 0), out_list, out_type);
    for (int i = 0; i < elm_count; i++) out_type[i] = c_fs_type_sector(out_type[i]);
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 2) { // file
            for (int j = 0; j < rec; j++) c_fskprint("  ");
            c_fskprint("| $6%s\n", out_list[i].name);
        }
    }
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 3) { // folder
            for (int j = 0; j < rec; j++) c_fskprint("  ");
            assemble_path(path, out_list[i].name, tmp_path);
            c_fskprint("%s\n", out_list[i].name);
            shell_tree(tmp_path, rec + 1);
        }
    }
    if (rec == 0) c_fskprint("\n");
    c_free(out_list);
    c_free(out_type);
}

void shell_cat(char fpath[], char suffix[]) {
    char *file = c_malloc(c_str_len(suffix) + c_str_len(fpath) + 2);
    assemble_path(fpath, suffix, file);
    if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
        uint32_t * file_content = c_fs_declare_read_array(file);
        char * char_content = c_fs_declare_read_array(file);
        c_fs_read_file(file, file_content);
        int char_count;
        for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
            char_content[char_count] = (char) file_content[char_count];
        char_content[char_count] = '\0';
        c_ckprint(char_content, c_magenta);
        c_fskprint("\n");
        c_free(file_content);
        c_free(char_content);
    } else c_fskprint("$3%s$B file not found\n", file);
    c_free(file);
}

void gpd() {
    for (int i = c_str_len(current_dir); i > 0; i--) {
        if (current_dir[i] == '/' || i == 1) {
            current_dir[i] = '\0';
            break;
        }
    }
}

int shell_command(char command[]) {
    char *prefix = c_malloc(c_str_len(command)); // size of char is 1 octet
    char *suffix = c_malloc(c_str_len(command));

    int ret = 0;

    c_str_cpy(prefix, command);
    c_str_cpy(suffix, command);
    c_str_start_split(prefix, ' ');
    c_str_end_split(suffix, ' ');

    if      (c_str_cmp(prefix, "cat") == 0)    shell_cat(current_dir, suffix);
    else if (c_str_cmp(prefix, "clear") == 0)  c_clear_screen();
    else if (c_str_cmp(prefix, "echo") == 0)   c_fskprint("$4%s\n", suffix);
    else if (c_str_cmp(prefix, "mkfile") == 0) c_fs_make_file(current_dir, suffix);
    else if (c_str_cmp(prefix, "reboot") == 0) c_sys_reboot();
    else if (c_str_cmp(prefix, "sc") == 0)     print_scancodes();
    else if (c_str_cmp(prefix, "sleep") == 0)  c_ms_sleep(c_ascii_to_int(suffix) * 1000);
    else if (c_str_cmp(prefix, "ss") == 0)     show_disk_LBA(suffix);
    else if (c_str_cmp(prefix, "stop") == 0)   c_sys_shutdown();
    else if (c_str_cmp(prefix, "tree") == 0)   shell_tree(current_dir, 0);
    else if (c_str_cmp(prefix, "yield") == 0)  c_yield((c_str_cmp(suffix, "yield") == 0) ? 1 : c_ascii_to_int(suffix));


    else if (c_str_cmp(prefix, "mkdir") == 0) {
        if (!c_str_cmp(suffix, "..")) {
            c_fskprint("$3Un dossier ne peut pas avoir comme nom .. !\n");
        } else {
            c_fs_make_dir(current_dir, suffix);
        }
    }

    else if (c_str_cmp(prefix, "wif") == 0) {
        // write in file
        char *file = c_malloc(c_str_len(suffix)+c_str_len(current_dir)+2);
        assemble_path(current_dir, suffix, file);
        if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
            char char_content[70];
            c_fskprint("-> "); c_input(char_content, 70, c_blue); c_fskprint("\n");
            uint32_t * file_content = c_malloc(c_str_len(char_content));
            for (int i = 0; i < 70; i++) file_content[i] = (uint32_t) char_content[i];
            c_fs_write_in_file(file, file_content, c_str_len(char_content));
            c_free(file_content);
        } else c_fskprint("$3%s$B file not found\n", file);
        c_free(file);
    }

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
