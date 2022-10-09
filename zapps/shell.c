#include "syscall.h"

#define BFR_SIZE 90
#define SC_MAX 57
#define LARGEUR c_vga_get_width()
#define HAUTEUR c_vga_get_height()

static char current_dir[256] = "/";

void parse_path(char path[], string_20_t liste_path[]);
int shell_command(char command[]);
void gpd();

int main(int arg) {

    char char_buffer[BFR_SIZE], last_buffer[BFR_SIZE];
    last_buffer[0] = '\0';
    while (1) {
        c_fskprint("profanOS [$9%s$7] -> ", current_dir);
        c_input_paste(char_buffer, BFR_SIZE, last_buffer, c_blue);
        c_str_cpy(last_buffer, char_buffer);
        c_fskprint("\n");
        if (shell_command(char_buffer)) break;
        char_buffer[0] = '\0';
    }
    return arg;
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

void print_time() {

    c_fskprint("$4UTC time:   ");
    time_t time;
    c_time_get(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        c_fskprint("$1%s$7:", tmp);
    }
    c_kprint_backspace(); c_fskprint(" ");
    for (int i = 3; i < 6; i++) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        c_fskprint("$1%s$7/", tmp);
    }
    c_kprint_backspace();
    c_fskprint("$4\nunix time:  $1%d\n", c_time_calc_unix(&time));
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

void usage() {

    int refresh_time[5], lvl[3] = {10, 5, 2};
    ScreenColor colors[3] = {c_dred, c_red, c_yellow};
    c_kprint(" ");
    
    c_timer_get_refresh_time(refresh_time);
    for (int ligne = 0; ligne < 3; ligne++) {
        for (int i = 0; i < 5; i++) {
            if (refresh_time[i] >= lvl[ligne]) c_ckprint("#", colors[ligne]);
            else c_kprint(" ");
        }
        c_kprint("\n ");
    }
    for (int i = 0; i < 5; i++) {
        if (refresh_time[i] < 10 && refresh_time[i] >= 0)
            c_fskprint("$1%d", refresh_time[i]);
        else c_ckprint("#", c_green);
    }
    c_kprint("\n");
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
    else if (c_str_cmp(prefix, "exit") == 0)   ret++;
    else if (c_str_cmp(prefix, "gpd") == 0)    gpd();
    else if (c_str_cmp(prefix, "help") == 0)   shell_cat("/", "zada/shell_help.txt");
    else if (c_str_cmp(prefix, "ls") == 0)     shell_ls((c_str_cmp(suffix, "ls") == 0) ? "" : suffix);
    else if (c_str_cmp(prefix, "mem") == 0)    c_mem_print();
    else if (c_str_cmp(prefix, "mkfile") == 0) c_fs_make_file(current_dir, suffix);
    else if (c_str_cmp(prefix, "reboot") == 0) c_sys_reboot();
    else if (c_str_cmp(prefix, "sc") == 0)     print_scancodes();
    else if (c_str_cmp(prefix, "sleep") == 0)  c_ms_sleep(c_ascii_to_int(suffix) * 1000);
    else if (c_str_cmp(prefix, "ss") == 0)     show_disk_LBA(suffix);
    else if (c_str_cmp(prefix, "stop") == 0)   c_sys_shutdown();
    else if (c_str_cmp(prefix, "tree") == 0)   shell_tree(current_dir, 0);
    else if (c_str_cmp(prefix, "usg") == 0)    usage();
    else if (c_str_cmp(prefix, "yield") == 0)  c_yield((c_str_cmp(suffix, "yield") == 0) ? 1 : c_ascii_to_int(suffix));


    else if (c_str_cmp(prefix, "mkdir") == 0) {
        if (!c_str_cmp(suffix, "..")) {
            c_fskprint("$3Un dossier ne peut pas avoir comme nom .. !\n");
        } else {
            c_fs_make_dir(current_dir, suffix);
        }
    }

    else if (c_str_cmp(prefix, "alloc") == 0) {
        if (suffix[0] == 'a') c_fskprint("$3size is required\n");
        else {
            int maddr = c_mem_alloc(c_ascii_to_int(suffix) * 1024);
            c_fskprint("$4address: $1%x $4($1%d$4)\n", maddr, maddr);
        }
    }

    else if (c_str_cmp(prefix, "udisk") == 0) {
        c_fskprint("disk scan in progress...\n");
        uint32_t sectors_count = c_ata_get_sectors_count();
        uint32_t used_sectors = c_fs_get_used_sectors(sectors_count);
        c_fskprint("$4total sector count: $1%d\n", sectors_count);
        c_fskprint("$4used sector count:  $1%d\n", used_sectors);
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

    else if (c_str_cmp(prefix, "go") == 0) {
        if(!(c_str_count(suffix, '.'))) c_str_cat(suffix, ".bin");
        char *file = c_malloc(c_str_len(suffix)+c_str_len(current_dir)+2);
        assemble_path(current_dir, suffix, file);
        if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2)
            c_sys_run_binary(file, 0);
        else c_fskprint("$3%s$B file not found\n", file);
        c_free(file);
    }

    else if (c_str_cmp(prefix, "info") == 0) {
        uint32_t sectors_count = c_ata_get_sectors_count();
        print_time();
        c_fskprint("$4ticks:      $1%d\n", c_timer_get_tick());
        c_fskprint("$4work time:  $1%ds$7/$1%ds\n", c_time_gen_unix() - c_time_get_boot() - c_timer_get_tick() / 100, c_time_gen_unix() - c_time_get_boot());
        c_fskprint("$4used mem:   $1%d%c\n", 100 * c_mem_get_usage() / c_mem_get_usable(), '%');
        c_fskprint("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_alloc_count() - c_mem_get_free_count(), c_mem_get_alloc_count());
        c_fskprint("$4disk size:  $1%d.%dMo\n", sectors_count / 2048, (sectors_count % 2048) / 20);
        c_task_print();
    }

    else if (c_str_cmp(prefix, "cd") == 0) {
        char old_path[256];
        c_str_cpy(old_path, current_dir);
        string_20_t * liste_path = c_calloc(1024);
        parse_path(suffix, liste_path);
        for (int i=0; i<c_str_count(suffix, '/')+1; i++) {
            if (!c_str_cmp(liste_path[i].name, "..")) {
                gpd();
            } else {
                char *new_path = c_calloc(256);
                assemble_path(current_dir, liste_path[i].name, new_path);
                if (c_fs_does_path_exists(new_path) && c_fs_type_sector(c_fs_path_to_id(new_path, 0)) == 3)
                    c_str_cpy(current_dir, new_path);
                else {
                    c_fskprint("$3%s$B path not found\n", new_path);
                    c_str_cpy(current_dir, old_path);
                    c_free(liste_path);
                    c_free(new_path);
                    break;
                }
                c_free(new_path);
            }
        }
        c_free(liste_path);
    }

    else if (c_str_cmp(prefix, "free") == 0) {
        if (c_mem_free_addr(c_ascii_to_int(suffix))) c_fskprint("$4free: $1OK\n");
        else c_fskprint("$4free: $3FAIL\n");
    }

    else if (c_str_cmp(prefix, "show") == 0) {
        char *file = c_malloc(c_str_len(suffix) + c_str_len(current_dir) + 2);
        assemble_path(current_dir, suffix, file);
        if (c_fs_does_path_exists(file) && c_fs_type_sector(c_fs_path_to_id(file, 0)) == 2) {
            uint32_t * file_content = c_fs_declare_read_array(file);
            char * char_content = c_fs_declare_read_array(file);
            c_fs_read_file(file, file_content);
            int char_count;
            for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
                char_content[char_count] = (char) file_content[char_count];
            char_content[char_count] = '\0';

            int index_char = 0;
            char size_hauteur[4];
            int index_hauteur = 0;
            char size_longueur[4];
            int index_longueur = 0;
            while (char_content[index_char] != '|') {
                size_hauteur[index_hauteur] = char_content[index_char];
                index_hauteur++; index_char++;
            } size_hauteur[index_hauteur] = '\0';
            index_char++;
            while (char_content[index_char] != '|') {
                size_longueur[index_longueur] = char_content[index_char];
                index_longueur++; index_char++;
            } size_longueur[index_longueur] = '\0';
            index_char++;

            int hauteur = c_ascii_to_int(size_hauteur);
            int longueur = c_ascii_to_int(size_longueur);
            c_vga_640_mode();

            char couleur_str[3];
            int index_couleur = 0;
            int couleur = 0;
            for (int h = 0; h<hauteur; h++) {
                for (int l = 0; l<longueur; l++) {
                    for (int i=0; i<3; i++) {couleur_str[i] = '\0';}
                    index_couleur = 0;
                    while (char_content[index_char] != '|') {
                        couleur_str[index_couleur] = char_content[index_char];
                        index_couleur++; index_char++;
                    } couleur_str[index_couleur] = '\0'; index_char++;
                    couleur = c_ascii_to_int(couleur_str);
                    c_vga_put_pixel(l, h, couleur);
                }
            }

            while (c_kb_get_scancode() != 1);
            c_vga_text_mode();

            c_fskprint("Hauteur : %d\n", hauteur);
            c_fskprint("Longueur : %d\n", longueur);

            c_free(file_content);
            c_free(char_content);
        } else c_fskprint("$3%s$B file not found\n", file);
        c_free(file);
    }

    else if (c_str_cmp(prefix, "") != 0) {
        c_fskprint("$3%s$B is not a valid command.\n", prefix);
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
