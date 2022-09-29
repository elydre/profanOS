#include "addf.h"

#define BFR_SIZE 90
#define SC_MAX 57

static char current_dir[256] = "/";

int shell_command(int addr, char command[]);

int start(int addr, int arg) {
    INIT_AF(addr);
    
    AF_input_paste();
    AF_fskprint();
    AF_str_cpy();

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
    return arg;
}

void assemble_path(int addr, char old[], char new[], char result[]) {
    INIT_AF(addr);
    
    AF_str_append();
    AF_str_cpy();
    AF_str_len();

    result[0] = '\0'; str_cpy(result, old);
    if (result[str_len(result) - 1] != '/') str_append(result, '/');
    for (int i = 0; i < str_len(new); i++) str_append(result, new[i]);
}

void shell_ls(int addr) {
    INIT_AF(addr);
    
    AF_fs_get_folder_size();
    AF_fs_get_dir_content();
    AF_fs_get_file_size();
    AF_fs_type_sector();
    AF_fs_path_to_id();
    AF_fskprint();
    AF_str_len();
    AF_malloc();
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

void print_scancodes(int addr) {
    INIT_AF(addr);
    
    AF_kb_scancode_to_char();
    AF_kb_get_scancode();
    AF_rainbow_print();
    AF_clear_screen();
    AF_fskprint();

    clear_screen();
    rainbow_print("enter scancode press ESC to exit\n");
    int last_sc = 0;
    while (1) {
        while (last_sc == kb_get_scancode());
        last_sc = kb_get_scancode();
        fskprint("$4\nscancode:   $1%d", last_sc);

        if (last_sc == 1) {
            // scancode 1 is the escape key
            clear_screen();
            break;
        }

        if (last_sc > SC_MAX) {
            fskprint("\n$5unknown scancode\n");
            continue;
        }

        fskprint("$4\nletter min: $1%c", kb_scancode_to_char(last_sc, 0));
        fskprint("$4\nletter maj: $1%c\n", kb_scancode_to_char(last_sc, 1));
    }
}

void print_time(int addr) {
    INIT_AF(addr);

    AF_kprint_backspace();
    AF_time_calc_unix();
    AF_int_to_ascii();
    AF_fskprint();
    AF_time_get();

    fskprint("$4UTC time:   ");
    time_t time;
    time_get(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fskprint("$1%s$7:", tmp);
    }
    kprint_backspace(); fskprint(" ");
    for (int i = 3; i < 6; i++) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fskprint("$1%s$7/", tmp);
    }
    kprint_backspace();
    fskprint("$4\nunix time:  $1%d\n", time_calc_unix(&time));
}

void show_disk_LBA(int addr, char suffix[]) {
    INIT_AF(addr);
    
    AF_ata_read_sector();
    AF_ascii_to_int();
    AF_int_to_ascii();
    AF_str_cmp();
    AF_ckprint();
    AF_str_len();
    AF_kprint();

    int LBA = 0;
    if (str_cmp(suffix, "ss") != 0) LBA = ascii_to_int(suffix);
    uint32_t outbytes[128];
    char tmp[10];
    ata_read_sector((uint32_t) LBA, outbytes);
    for (int i = 0; i < 128; i++) {
        int_to_ascii(outbytes[i], tmp);
        ckprint(tmp, c_magenta);
        for (int j = str_len(tmp); j < 10; j++) kprint(" ");
    }
}

void shell_tree(int addr, char path[], int rec) {
    INIT_AF(addr);

    AF_fs_get_dir_content();
    AF_fs_get_folder_size();
    AF_fs_type_sector();
    AF_fs_path_to_id();
    AF_fskprint();
    AF_malloc();
    AF_free();

    int elm_count = fs_get_folder_size(path);
    string_20_t *out_list = malloc(elm_count * sizeof(string_20_t));
    uint32_t *out_type = malloc(elm_count * sizeof(uint32_t));
    char tmp_path[256];
    fs_get_dir_content(fs_path_to_id(path, 0), out_list, out_type);
    for (int i = 0; i < elm_count; i++) out_type[i] = fs_type_sector(out_type[i]);
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 2) { // file
            for (int j = 0; j < rec; j++) fskprint("  ");
            fskprint("| $6%s\n", out_list[i].name);
        }
    }
    for (int i = 0; i < elm_count; i++) {
        if (out_type[i] == 3) { // folder
            for (int j = 0; j < rec; j++) fskprint("  ");
            assemble_path(addr, path, out_list[i].name, tmp_path);
            fskprint("%s\n", out_list[i].name);
            shell_tree(addr, tmp_path, rec + 1);
        }
    }
    if (rec == 0) fskprint("\n");
    free(out_list);
    free(out_type);
}

void shell_cat(int addr, char fpath[], char suffix[]) {
    INIT_AF(addr);

    AF_fs_declare_read_array();
    AF_fs_does_path_exists();
    AF_fs_type_sector();
    AF_fs_path_to_id();
    AF_fs_read_file();
    AF_fskprint();
    AF_str_len();
    AF_ckprint();
    AF_malloc();
    AF_free();

    char *file = malloc(str_len(suffix) + str_len(fpath) + 2);
        assemble_path(addr, fpath, suffix, file);
        if (fs_does_path_exists(file) && fs_type_sector(fs_path_to_id(file, 0)) == 2) {
            uint32_t * file_content = fs_declare_read_array(file);
            char * char_content = fs_declare_read_array(file);
            fs_read_file(file, file_content);
            int char_count;
            for (char_count = 0; file_content[char_count] != (uint32_t) -1; char_count++)
                char_content[char_count] = (char) file_content[char_count];
            char_content[char_count] = '\0';
            ckprint(char_content, c_magenta);
            fskprint("\n");
            free(file_content);
            free(char_content);
        } else fskprint("$3%s$B file not found\n", file);
        free(file);
}

void usage(int addr) {
    INIT_AF(addr);

    AF_timer_get_refresh_time();
    AF_fskprint();
    AF_ckprint();
    AF_kprint();

    int refresh_time[5], lvl[3] = {10, 5, 2};
    ScreenColor colors[3] = {c_dred, c_red, c_yellow};
    kprint(" ");
    
    timer_get_refresh_time(refresh_time);
    for (int ligne = 0; ligne < 3; ligne++) {
        for (int i = 0; i < 5; i++) {
            if (refresh_time[i] >= lvl[ligne]) ckprint("#", colors[ligne]);
            else kprint(" ");
        }
        kprint("\n ");
    }
    for (int i = 0; i < 5; i++) {
        if (refresh_time[i] < 10 && refresh_time[i] >= 0)
            fskprint("$1%d", refresh_time[i]);
        else ckprint("#", c_green);
    }
    kprint("\n");
}

void gpd(int addr) {
    INIT_AF(addr);
    AF_str_len();

    for (int i = str_len(current_dir); i > 0; i--) {
        if (current_dir[i] == '/' || i == 1) {
            current_dir[i] = '\0';
            break;
        }
    }
}

int shell_command(int addr, char command[]) {
    INIT_AF(addr);
    
    AF_ata_get_sectors_count();
    AF_fs_does_path_exists();
    AF_fs_get_used_sectors();
    AF_mem_get_alloc_count();
    AF_mem_get_free_count();
    AF_fs_write_in_file();
    AF_str_start_split();
    AF_fs_type_sector();
    AF_sys_run_binary();
    AF_timer_get_tick();
    AF_mem_get_usable();
    AF_time_gen_unix();
    AF_mem_get_usage();
    AF_str_end_split();
    AF_time_get_boot();
    AF_fs_path_to_id();
    AF_mem_free_addr();
    AF_clear_screen();
    AF_ascii_to_int();
    AF_fs_make_file();
    AF_sys_shutdown();
    AF_fs_make_dir();
    AF_sys_reboot();
    AF_task_print();
    AF_mem_print();
    AF_str_count();
    AF_mem_alloc();
    AF_ms_sleep();
    AF_fskprint();
    AF_str_cpy();
    AF_str_len();
    AF_str_cmp();
    AF_str_cat();
    AF_malloc();
    AF_input();
    AF_yield();
    AF_free();


    char *prefix = malloc(str_len(command)); // size of char is 1 octet
    char *suffix = malloc(str_len(command));

    int ret = 0;

    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "cat") == 0)    shell_cat(addr, current_dir, suffix);
    else if (str_cmp(prefix, "clear") == 0)  clear_screen();
    else if (str_cmp(prefix, "echo") == 0)   fskprint("$4%s\n", suffix);
    else if (str_cmp(prefix, "exit") == 0)   ret++;
    else if (str_cmp(prefix, "gpd") == 0)    gpd(addr);
    else if (str_cmp(prefix, "help") == 0)   shell_cat(addr, "/", "user/shell_help.txt");
    else if (str_cmp(prefix, "ls") == 0)     shell_ls(addr);
    else if (str_cmp(prefix, "mem") == 0)    mem_print();
    else if (str_cmp(prefix, "mkdir") == 0)  fs_make_dir(current_dir, suffix);
    else if (str_cmp(prefix, "mkfile") == 0) fs_make_file(current_dir, suffix);
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "sc") == 0)     print_scancodes(addr);
    else if (str_cmp(prefix, "sleep") == 0)  ms_sleep(ascii_to_int(suffix) * 1000);
    else if (str_cmp(prefix, "ss") == 0)     show_disk_LBA(addr, suffix);
    else if (str_cmp(prefix, "stop") == 0)   sys_shutdown();
    else if (str_cmp(prefix, "tree") == 0)   shell_tree(addr, current_dir, 0);
    else if (str_cmp(prefix, "usg") == 0)    usage(addr);
    else if (str_cmp(prefix, "yield") == 0)  (str_cmp(suffix, "yield") == 0) ? yield(1) : yield(ascii_to_int(suffix));

    else if (str_cmp(prefix, "alloc") == 0) {
        if (suffix[0] == 'a') fskprint("$3size is required\n");
        else {
            int maddr = mem_alloc(ascii_to_int(suffix) * 1024);
            fskprint("$4address: $1%x $4($1%d$4)\n", maddr, maddr);
        }
    }

    else if (str_cmp(prefix, "udisk") == 0) {
        fskprint("disk scan in progress...\n");
        uint32_t sectors_count = ata_get_sectors_count();
        uint32_t used_sectors = fs_get_used_sectors(sectors_count);
        fskprint("$4total sector count: $1%d\n", sectors_count);
        fskprint("$4used sector count:  $1%d\n", used_sectors);
    }

    else if (str_cmp(prefix, "wif") == 0) {
        // write in file
        char *file = malloc(str_len(suffix)+str_len(current_dir)+2);
        assemble_path(addr, current_dir, suffix, file);
        if (fs_does_path_exists(file) && fs_type_sector(fs_path_to_id(file, 0)) == 2) {
            char char_content[70];
            fskprint("-> "); input(char_content, 70, c_blue); fskprint("\n");
            uint32_t * file_content = malloc(str_len(char_content));
            for (int i = 0; i < 70; i++) file_content[i] = (uint32_t) char_content[i];
            fs_write_in_file(file, file_content, str_len(char_content));
            free(file_content);
        } else fskprint("$3%s$B file not found\n", file);
        free(file);
    }

    else if (str_cmp(prefix, "go") == 0) {
        if(!(str_count(suffix, '.'))) str_cat(suffix, ".bin");
        char *file = malloc(str_len(suffix)+str_len(current_dir)+2);
        assemble_path(addr, current_dir, suffix, file);
        fskprint("file : %s\n", file);
        if (fs_does_path_exists(file) && fs_type_sector(fs_path_to_id(file, 0)) == 2) {
            sys_run_binary(file, 0);
        } else fskprint("$3%s$B file not found\n", file);
        free(file);
    }

    else if (str_cmp(prefix, "info") == 0) {
        uint32_t sectors_count = ata_get_sectors_count();
        print_time(addr);
        fskprint("$4ticks:      $1%d\n", timer_get_tick());
        fskprint("$4work time:  $1%ds$7/$1%ds\n", time_gen_unix() - time_get_boot() - timer_get_tick() / 100, time_gen_unix() - time_get_boot());
        fskprint("$4used mem:   $1%d%c\n", 100 * mem_get_usage() / mem_get_usable(), '%');
        fskprint("$4act alloc:  $1%d$7/$1%d\n", mem_get_alloc_count() - mem_get_free_count(), mem_get_alloc_count());
        fskprint("$4disk size:  $1%d.%dMo\n", sectors_count / 2048, (sectors_count % 2048) / 20);
        task_print();
    }

    else if (str_cmp(prefix, "cd") == 0) {
        char new_path[256];
        assemble_path(addr, current_dir, suffix, new_path);
        if (fs_does_path_exists(new_path) && fs_type_sector(fs_path_to_id(new_path, 0)) == 3)
            str_cpy(current_dir, new_path);
        else fskprint("$3%s$B path not found\n", new_path);
    }

    else if (str_cmp(prefix, "free") == 0) {
        if (mem_free_addr(ascii_to_int(suffix))) fskprint("$4free: $1OK\n");
        else fskprint("$4free: $3FAIL\n");
    }

    else if (str_cmp(prefix, "") != 0) {
        fskprint("$3%s$B is not a valid command.\n", prefix);
    }

    free(prefix);
    free(suffix);
    return ret;
}
