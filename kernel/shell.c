#include "../drivers/screen.h"
#include "../libc/string.h"
#include "shell.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
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

    else if (strcmp(prefix, "") != 0) {
        ckprint(prefix, c_red);
        ckprint(" is not a valid command.\n", c_dred);
    }
    if (strcmp(prefix, "") * strcmp(prefix, "CLEAR") != 0) { kprint("\n"); }

    shell_omp();
}
