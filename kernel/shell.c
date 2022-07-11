#include "../drivers/screen.h"
#include "../libc/string.h"
#include "../drivers/ata/ata.h"
#include "kernel.h"
#include "shell.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
}

void shell_help(char suffix[]) {
    char *help[] = {
        "CLEAR   - clear the screen",
        "ECHO    - print the arguments",
        "END     - shutdown the system",
        "HELP    - show this help",
        "VERSION - display the version",
    };

    if (strcmp(suffix, "HELP") == 0) {
        for (int i = 0; i < 5; i++) {       // TODO: len of list
            ckprint(help[i], c_magenta);
            kprint("\n");
        }
    } else {
        char tmp[100];                      // TODO: autolen
        for (int i = 0; i < 5; i++) {
            strcpy(tmp, help[i]);
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

void shell_command(char *command) {
    char prefix[strlen(command)], suffix[strlen(command)];
    strcpy(prefix, command);
    strcpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');
    
    if (strcmp(prefix, "END") == 0) {
        rainbow_print("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }

    else if (strcmp(prefix, "ECHO") == 0) {
        ckprint(suffix, c_magenta);
        kprint("\n");
    }

    else if (strcmp(prefix, "TD") == 0) {
        write_sectors_ATA_PIO(1, 1, (uint32_t*)0xB8000);
        ckprint("done\n", c_magenta);
    }

    else if (strcmp(prefix, "CLEAR") == 0) {
        clear_screen();
    }

    else if (strcmp(prefix, "HELP") == 0) {
        shell_help(suffix);
    }

    else if (strcmp(prefix, "VERSION") == 0) {
            ckprint("version ", c_magenta);
            ckprint(VERSION, c_magenta);
            kprint("\n");
    }

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }
    if (strcmp(prefix, "") * strcmp(prefix, "CLEAR") != 0) { kprint("\n"); }

    shell_omp();
}
