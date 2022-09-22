#include <driver/keyboard.h>
#include <driver/screen.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <driver/ata.h>
#include <filesystem.h>
#include <cpu/timer.h>
#include <function.h>
#include <string.h>
#include <system.h>
#include "kernel.h"
#include <iolib.h>
#include <time.h>
#include <task.h>
#include <mem.h>

#define SC_MAX 57
#define BFR_SIZE 68

static char current_dir[256] = "/";

void shell_omp() {
    fskprint("profanOS [$9%s$7] -> ", current_dir);
}

void print_scancodes() {
    clear_screen();
    rainbow_print("enter scancode press ESC to exit\n");
    int last_sc = 0;
    while (1) {
        while (last_sc == get_last_scancode());
        last_sc = get_last_scancode();
        fskprint("$4\nscancode:   $1%d", last_sc);

        if (last_sc == 1) {
            // scancode 1 is the escape key
            clear_screen();
            return;
        }

        if (last_sc > SC_MAX) {
            ckprint("\nunknown scancode\n", c_yellow);
            continue;
        }

        fskprint("$4\nletter min: $1%c", scancode_to_char(last_sc, 0));
        fskprint("$4\nletter maj: $1%c\n", scancode_to_char(last_sc, 1));
    }
}

void show_disk_LBA(char suffix[]) {
    int LBA = 0;
    if (strcmp(suffix, "ss") != 0) LBA = ascii_to_int(suffix);
    uint32_t outbytes[128];
    char tmp[10];
    read_sectors_ATA_PIO((uint32_t) LBA, outbytes);
    for (int i = 0; i < 128; i++) {
        int_to_ascii(outbytes[i], tmp);
        ckprint(tmp, c_magenta);
        for (int j = strlen(tmp); j < 10; j++) kprint(" ");
    }
}

void shell_help(char suffix[]) {
    char *help[] = {
        "alloc   - allocate *suffix* ko",
        "clear   - clear the screen",
        "echo    - print the arguments",
        "free    - free *suffix* address",
        "help    - show this help",
        "info    - show time, task & page info",
        "mem     - show MLIST with colors",
        "reboot  - reboot the system",
        "sc      - show the scancodes",
        "sleep   - sleep for a given time",
        "ss      - show int32 in the LBA *suffix*",
        "stop    - shutdown the system",
        "usg     - show the usage of cpu",
        "ver     - display the version",
        "yield   - yield to pid *suffix*"
    };

    if (strcmp(suffix, "help") == 0) {
        for (int i = 0; i < ARYLEN(help); i++) {
            mskprint(3, "$4", help[i], "\n");
        }
    } else {
        char tmp[100];
        for (int i = 0; i < ARYLEN(help); i++) {
            strcpy_s(tmp, help[i]);
            str_start_split(tmp, ' ');

            if (strcmp(tmp, suffix) == 0) {
                mskprint(3, "$4", help[i], "\n");
                return;
            }
        }
        fskprint("$1%s$4 not found in help\n", suffix);
    }
}

void print_time() {
    ckprint("UTC time:   ", c_magenta);
    time_t time;
    get_time(&time);
    char tmp[12];
    for (int i = 2; i >= 0; i--) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        mskprint(3, "$1", tmp, "$7:");
    }
    kprint_backspace(); kprint(" ");
    for (int i = 3; i < 6; i++) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        ckprint(tmp, c_green);
        kprint("/");
    }
    kprint_backspace();
    int_to_ascii(calc_unix_time(&time), tmp);
    mskprint(3, "$4\nunix time:  $1", tmp, "\n\n");
}

void usage() {
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
        if (refresh_time[i] < 10 && refresh_time[i] >= 0) {
            fskprint("$1%d", refresh_time[i]);
        } else {
            ckprint("#", c_green);
        }
    }
    kprint("\n");
}

void shell_command(char command[]) {
    char *prefix = malloc(strlen(command)); // size of char is 1 octet
    char *suffix = malloc(strlen(command));
    strcpy(prefix, command);
    strcpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (strcmp(prefix, "clear") == 0)  clear_screen();
    else if (strcmp(prefix, "echo") == 0)   mskprint(3, "$4", suffix, "\n");
    else if (strcmp(prefix, "help") == 0)   shell_help(suffix);
    else if (strcmp(prefix, "mem") == 0)    memory_print();
    else if (strcmp(prefix, "mkdir") == 0) make_dir(current_dir, suffix);
    else if (strcmp(prefix, "mkfile") == 0) make_file(current_dir, suffix);
    else if (strcmp(prefix, "reboot") == 0) sys_reboot();
    else if (strcmp(prefix, "sc") == 0)     print_scancodes();
    else if (strcmp(prefix, "sleep") == 0)  ms_sleep(ascii_to_int(suffix) * 1000);
    else if (strcmp(prefix, "ss") == 0)     show_disk_LBA(suffix);
    else if (strcmp(prefix, "usg") == 0)    usage();
    else if (strcmp(prefix, "ver") == 0)    mskprint(3, "$4version ", VERSION, "\n");
    else if (strcmp(prefix, "yield") == 0)  (strcmp(suffix, "yield") == 0) ? yield(1) : yield(ascii_to_int(suffix));

    else if (strcmp(prefix, "alloc") == 0) {
        if (suffix[0] == 'a') fskprint("$3size is required\n");
        else {
            int addr = alloc(ascii_to_int(suffix) * 1024);
            fskprint("$4address: $1%x $4($1%d$4)\n", addr, addr);
        }
    }

    else if (strcmp(prefix, "stop") * strcmp(prefix, "exit") == 0) {
        rainbow_print("Stopping the CPU. Bye!\n");
        sys_shutdown();
    }

    else if (strcmp(prefix, "ls") == 0) {
        int elm_count = get_folder_size(current_dir);
        string_20_t *out_list = malloc(elm_count * sizeof(string_20_t));
        uint32_t *out_type = malloc(elm_count * sizeof(uint32_t));
        get_dir_content(path_to_id(current_dir, 0), out_list, out_type);
        for (int i = 0; i < elm_count; i++) out_type[i] = type_sector(out_type[i]);
        for (int i = 0; i < 10; i++) {
            if (out_list[i].name[0] == '\0') break;
            if (out_type[i] == 2) fskprint("$1%s\n", out_list[i].name);
            else if (out_type[i] == 3) fskprint("$2%s\n", out_list[i].name);
            else fskprint("$3%s\n", out_list[i].name);
        }
        free((int) out_list);
    }

    else if (strcmp(prefix, "udisk") == 0) {
        fskprint("disk scan in progress...\n");
        uint32_t sectors_count = get_ATA_sectors_count();
        uint32_t used_sectors = get_used_sectors(sectors_count);
        fskprint("$4total sector count: $1%d\n", sectors_count);
        fskprint("$4used sector count:  $1%d\n", used_sectors);
    }

    else if (strcmp(prefix, "gpd") == 0) {
        // got to parent directory
        for (int i = strlen(current_dir); i > 0; i--) {
            if (current_dir[i] == '/' || i == 1) {
                current_dir[i] = '\0';
                break;
            }
        }
    }

    else if (strcmp(prefix, "cd") == 0) {
        char new_path[256];
        strcpy(new_path, current_dir);
        if (current_dir[strlen(current_dir) - 1] != '/') append(new_path, '/');
        for (int i = 0; i < strlen(suffix); i++) append(new_path, suffix[i]);
        if (does_path_exists(new_path)) strcpy(current_dir, new_path);
        else fskprint("$3%s$B path not found\n", new_path);
    }

    else if (strcmp(prefix, "free") == 0) {
        if (free(ascii_to_int(suffix))) mskprint(2, "$4free: $1", "OK\n");
        else mskprint(2, "$4free: $3", "FAIL\n");
    }

    else if (strcmp(prefix, "info") == 0) {
        uint32_t sectors_count = get_ATA_sectors_count();

        print_time();
        fskprint("$4ticks:      $1%d\n", timer_get_tick());
        fskprint("$4on time:    $1%ds\n", gen_unix_time() - get_boot_time());
        fskprint("$4work time:  $1%ds\n\n", gen_unix_time() - get_boot_time() - timer_get_tick() / 100);
        fskprint("$4used mem:   $1%d%c\n", 100 * get_memory_usage() / get_usable_memory(), '%');
        fskprint("$4disk size:  $1%d.%dMo\n\n", sectors_count / 2048, (sectors_count % 2048) / 20);
        task_printer();
    }

    else if (strcmp(prefix, "") != 0) {
        mskprint(3, "$3", prefix, "$B is not a valid command.\n");
    }

    free((int) prefix);
    free((int) suffix);
}

void run_shell() {
    char char_buffer[BFR_SIZE], last_buffer[BFR_SIZE];
    last_buffer[0] = '\0';
    while (1) {
        shell_omp();
        input_paste(char_buffer, BFR_SIZE, last_buffer, c_blue);
        strcpy(last_buffer, char_buffer);
        kprint("\n");
        shell_command(char_buffer);
        char_buffer[0] = '\0';
    }
}
