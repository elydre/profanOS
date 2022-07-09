#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "shell.h"
#include <stdint.h>

static char key_buffer[256];


void kernel_main() {
    isr_install();
    irq_install();

    asm("int $2");
    asm("int $3");

    rainbow_print("\n\nWelcome to profanOS!\n\n");
    shell_omp();
}


void user_input(char letter, int is_enter, int is_backspace) {
    // shell mode
    if (is_backspace ) {
        if (strlen(key_buffer)){
            backspace(key_buffer);
            kprint_backspace();
        }
    }
    
    else if (is_enter) {
        kprint("\n");
        shell_command(key_buffer);
        key_buffer[0] = '\0';
    }
    
    else {
        char str[2] = {letter, '\0'};
        append(key_buffer, letter);
        ckprint(str, c_blue);
    }
}
