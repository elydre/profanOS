#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../drivers/ata/ata.h"
#include "../drivers/rtc.h"
#include "../libc/string.h"
#include "../libc/function.h"
#include "../libc/mem.h"
#include "../libc/time.h"
#include "../cpu/timer.h"
#include "kernel.h"
#include "shell.h"
#include "task.h"
#include <stdint.h>

#define SC_MAX 57

void shell_omp(){
    kprint("profanOS-> ");
}

void disk_test() {
    uint32_t inbytes[128];
    uint32_t outbytes[128];
    int sum = 0;
    char tmp[3];

    ckprint("writing to disk\n", c_magenta);
    
    for (int i = 0; i < 128; i++) inbytes[i] = i;

    write_sectors_ATA_PIO(0, inbytes);
    ckprint("writing done\nreading from disk\n", c_magenta);
    
    read_sectors_ATA_PIO(0, outbytes);

    for (int i = 0; i < 8; i++) {
        sum += outbytes[i];
        int_to_ascii(outbytes[i], tmp);
        ckprint(tmp, c_magenta);
        kprint("\n");
    }

    if (sum == 28) ckprint("...\nsum == 28, it works!\n", c_magenta);
    else ckprint("...\nsum != 28, it doesn't work!\n", c_red);
}

void print_scancodes() {
    clear_screen();
    rainbow_print("enter scancode press ESC to exit\n");
    int last_sc = 0;
    char str[10];
    while (1) {
        while (last_sc == get_last_scancode());
        last_sc = get_last_scancode();
        ckprint("\nscancode: ", c_blue);
        int_to_ascii(last_sc, str);
        ckprint(str, c_dcyan);

        if (last_sc == 1) {
            // scancode 1 is the escape key
            clear_screen();
            return;
        }

        if (last_sc > SC_MAX) {
            ckprint("\nunknown scancode\n", c_yellow);
            continue;
        }

        str[0] = scancode_to_char(last_sc, 0);
        str[1] = '\0';
        ckprint("\nletter min: ", c_blue);
        ckprint(str, c_dcyan);
        str[0] = scancode_to_char(last_sc, 1);
        ckprint("\nletter maj: ", c_blue);
        ckprint(str, c_dcyan);
        kprint("\n");
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

void page_info(){
    uint32_t page = alloc_page(1);
    ckprint("actual page address: ", c_magenta);
    char tmp[11];
    hex_to_ascii(page, tmp);
    ckprint(tmp, c_magenta);
    kprint("\n");
}

void shell_help(char suffix[]) {
    char *help[] = {
        "CLEAR   - clear the screen",
        "ECHO    - print the arguments",
        "END     - shutdown the system",
        "HELP    - show this help",
        "PAGE    - show the actual page address",
        "SC      - show the scancodes",
        "SLEEP   - sleep for a given time",
        "SS      - show int32 in the LBA *suffix*",
        "TASK    - print info about the tasks",
        "TIME    - print the GMT from the RTC",
        "TD      - test the disk",
        "USG     - show the usage of cpu",
        "VER     - display the version",
        "YIELD   - yield to the next task",
    };

    if (strcmp(suffix, "help") == 0) {
        for (int i = 0; i < ARYLEN(help); i++) {
            ckprint(help[i], c_magenta);
            kprint("\n");
        }

    } else {
        char tmp[100];
        for (int i = 0; i < ARYLEN(help); i++) {
            strcpy_s(tmp, help[i]);
            str_start_split(tmp, ' ');

            if (strcmp(tmp, suffix) == 0) {
                ckprint(help[i], c_magenta);
                kprint("\n");
                return;
            }
        }
        ckprint("command not found\n", c_red);
    }
}

void print_time() {
    ckprint("UTC: ", c_magenta);
    time_t time;
    get_time(&time);
    char tmp[12];
    for (int i = 2; i >= 0; i--) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        ckprint(tmp, c_magenta);
        kprint(":");
    }
    kprint_backspace(); kprint(" ");
    for (int i = 3; i < 6; i++) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        ckprint(tmp, c_magenta);
        kprint("/");
    }
    kprint_backspace(); kprint("\n");
    int_to_ascii(calc_unix_time(&time), tmp);
    ckprint("unix time: ", c_magenta);
    ckprint(tmp, c_magenta);
    kprint("\n");
}

void usage() {
    char tmp[2];
    int refresh_time[5];
    int lvl[3] = {10, 5, 2};
    ScreenColor colors[3] = {c_dred, c_red, c_yellow};
    
    timer_get_refresh_time(refresh_time);
    for (int ligne = 0; ligne < 3; ligne++) {
        for (int i = 0; i < 5; i++) {
            if (refresh_time[i] >= lvl[ligne]) ckprint("#", colors[ligne]);
            else kprint(" ");
        }
        kprint("\n");
    }
    for (int i = 0; i < 5; i++) {
        if (refresh_time[i] < 10) {
            int_to_ascii(refresh_time[i], tmp);
            ckprint(tmp, c_green);
        } else {
            ckprint("+", c_green);
        }
    }
    kprint("\n");
}

void shell_command(char command[]) {
    char prefix[strlen(command)], suffix[strlen(command)];
    strcpy(prefix, command);
    strcpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');
    
    if (strcmp(prefix, "clear") == 0) {
        clear_screen();
    }

    else if (strcmp(prefix, "echo") == 0) {
        ckprint(suffix, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "end") == 0) {
        rainbow_print("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }

    else if (strcmp(prefix, "help") == 0) {
        shell_help(suffix);
    }

    else if (strcmp(prefix, "page") == 0) {
        page_info();
    }

    else if (strcmp(prefix, "sc") == 0) {
        print_scancodes();
    }

    else if (strcmp(prefix, "sleep") == 0) {
        sleep(ascii_to_int(suffix));
    }

    else if (strcmp(prefix, "ss") == 0) {
        show_disk_LBA(suffix);
    }

    else if (strcmp(prefix, "task") == 0) {
        task_printer();
    }

    else if (strcmp(prefix, "time") == 0) {
        print_time();
    }

    else if (strcmp(prefix, "td") == 0) {
        disk_test();
    }

    else if (strcmp(prefix, "usg") == 0) {
        usage();
    }

    else if (strcmp(prefix, "ver") == 0) {
        ckprint("version ", c_magenta);
        ckprint(VERSION, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "yield") == 0) {
        yield();
    }

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }

    if (strcmp(prefix, "") *
        strcmp(prefix, "clear") *
        strcmp(prefix, "sleep") *
        strcmp(prefix, "sc")
    != 0) { kprint("\n"); }

    shell_omp();
}
