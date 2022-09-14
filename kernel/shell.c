#include <driver/keyboard.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <driver/ata.h>
#include <filesystem.h>
#include <cpu/timer.h>
#include <function.h>
#include <iolib.h>
#include <driver/screen.h>
#include "kernel.h"
#include <string.h>
#include <system.h>
#include <time.h>
#include <task.h>
#include <mem.h>

#define SC_MAX 57

void shell_omp() {
    kprint("profanOS-> ");
}

void disk_test() {
    uint32_t inbytes[128], outbytes[128];
    int sum = 0;

    ckprint("writing to disk\n", c_magenta);
    
    for (int i = 0; i < 128; i++) inbytes[i] = i;

    write_sectors_ATA_PIO(0, inbytes);
    ckprint("writing done\nreading from disk\n", c_magenta);
    
    read_sectors_ATA_PIO(0, outbytes);

    for (int i = 0; i < 10; i++) {
        sum += outbytes[i];
        fskprint("$1%d ", outbytes[i]);
    }

    if (sum == 45) ckprint("\n\nsum == 45, it works!\n", c_magenta);
    else ckprint("\nsum != 45, it doesn't work!\n", c_dred);
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
        "alloc   - allocate *suffix* octets",
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
        "td      - test the disk",
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
        ckprint("command not found\n", c_red);
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
    else if (strcmp(prefix, "reboot") == 0) sys_reboot();
    else if (strcmp(prefix, "sc") == 0)     print_scancodes();
    else if (strcmp(prefix, "sleep") == 0)  ms_sleep(ascii_to_int(suffix) * 1000);
    else if (strcmp(prefix, "ss") == 0)     show_disk_LBA(suffix);
    else if (strcmp(prefix, "td") == 0)     disk_test();
    else if (strcmp(prefix, "usg") == 0)    usage();
    else if (strcmp(prefix, "ver") == 0)    mskprint(3, "$4version ", VERSION, "\n");
    else if (strcmp(prefix, "yield") == 0)  (strcmp(suffix, "yield") == 0) ? yield(1) : yield(ascii_to_int(suffix));

    else if (strcmp(prefix, "alloc") == 0) {
        if (suffix[0] == 'a') fskprint("$3size is required\n");
        else {
            int addr = alloc(ascii_to_int(suffix));
            fskprint("$4address: $1%x $4($1%d$4)\n", addr, addr);
        }
    }

    else if (strcmp(prefix, "stop") == 0) {
        rainbow_print("Stopping the CPU. Bye!\n");
        sys_shutdown();
    }

    else if (strcmp(prefix, "test") == 0) {
        fskprint("fichier 0 : %d\n", i_creer_index_de_fichier("test"));
        i_set_data_to_file("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 200, 0);
        fskprint("fichier 1 : %d\n", i_creer_index_de_fichier("coucou"));
        i_set_data_to_file("12345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890", 200, 3);
    }

    else if (strcmp(prefix, "input") == 0) {
        char out[100];
        input(out, 100, c_yellow);
        fskprint("\n$4input: $1%s\n", out);
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

    if (strcmp(prefix, "") *
        strcmp(prefix, "clear") *
        strcmp(prefix, "sleep") *
        strcmp(prefix, "sc")
    != 0) { kprint("\n"); }

    free((int) prefix);
    free((int) suffix);
    shell_omp();
}
