#include "../drivers/screen.h"
#include "../libc/string.h"
#include "shell.h"
#include <stdint.h>

void shell_omp(){
    kprint("profanOS-> ");
}

void shell_command(char *command) {
    if (strcmp(command, "END") == 0) {
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }

    else if (strcmp(command, "TEST") == 0) {
        ckprint(return_int_to_ascii(42), c_magenta);
    }

    else if (strcmp(command, "CLEAR") == 0) {
        clear_screen();
    }

    else if (strcmp(command, "") != 0) { kprint("not found"); }
    if (strcmp(command, "") * strcmp(command, "CLEAR") != 0) { kprint("\n"); }

    shell_omp();
}
