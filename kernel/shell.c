#include "../drivers/screen.h"
#include "../libc/string.h"
#include "shell.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
}

void shell_help(char suffix[]) {
    char help[] = "HELP  - show this help\nCLEAR - clear the screen\nEND   - shutdown the system\nECHO  - print the arguments";

    if (strcmp(suffix, "HELP") == 0) {
            ckprint(help, c_magenta);
            kprint("\n");
    } else {
        char tmp[100];
        char rest[100];
        char bk[100];
        strcpy(tmp, help);
        strcpy(rest, help);
        for (int i = 0; i < 4; i++) {       // 4 is the number of
            str_start_split(tmp, '\n');     // commands in the help
            str_end_split(rest, '\n');
            strcpy(bk, tmp);
            str_start_split(tmp, ' ');
            if (strcmp(tmp, suffix) == 0) {
                ckprint(bk, c_magenta);
                kprint("\n");
                return;
            }
            strcpy(tmp, rest);
        }
        ckprint("command not found\n", c_red);
    }
}

void shell_command(char *command) {
    char prefix[strlen(command)], suffix[strlen(command)];
    strcpy(prefix, command);
    strcpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');
    
    if (strcmp(prefix, "END") == 0) {
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }

    else if (strcmp(prefix, "ECHO") == 0) {
        ckprint(suffix, c_magenta);
    }

    else if (strcmp(prefix, "CLEAR") == 0) {
        clear_screen();
    }

    else if (strcmp(prefix, "HELP") == 0) {
        shell_help(suffix);
    }

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }
    if (strcmp(prefix, "") * strcmp(prefix, "CLEAR") != 0) { kprint("\n"); }

    shell_omp();
}
