#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "shell.h"
#include <stdint.h>

#define BACKSPACE 14
#define ENTER 28
#define EXIT 1

static char key_buffer[256];
static char last_command[256];
static int mod = 0; // 0 = shell, 1 = scancode


void kernel_main() {
    isr_install();
    irq_install();

    asm("int $2");
    asm("int $3");

    rainbow_print("\n\nWelcome to profanOS!\n\n");
    shell_omp();
}

void shell_mod(char letter, int scancode) {
    if (scancode == BACKSPACE) {
        if (strlen(key_buffer)){
            backspace(key_buffer);
            kprint_backspace();
        }
    }
    
    else if (scancode == ENTER) {
        kprint("\n");
        strcpy(last_command, key_buffer);
        shell_command(key_buffer);
        key_buffer[0] = '\0';
    }

    else{
        char str[2] = {letter, '\0'};
        append(key_buffer, letter);
        ckprint(str, c_blue);
    }
}

void scancode_mod(char letter, int scancode) {
    if (scancode == EXIT) {
        clear_screen();
        shell_omp();
        mod = 0;
        return;
    }

    char str[3] = {letter, '\0', '\0'};
    ckprint("\nletter: ", c_dblue);
    ckprint(str, c_blue);
    ckprint("\nscancode: ", c_dblue);
    int_to_ascii(scancode, str);
    ckprint(str, c_blue);
    kprint("\n");
}

void user_input(char letter, int scancode) {
    if (scancode == EXIT && mod != 1) {
        clear_screen();
        rainbow_print("Entering scancode mode.\n");
        mod = 1;
    }

    else if (mod == 0) {
        shell_mod(letter, scancode);
    }
    
    else if (mod == 1) {
        scancode_mod(letter, scancode);
    }
}
