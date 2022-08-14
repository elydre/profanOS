#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "../drivers/keyboard.h"
#include "../drivers/rtc.h"
#include "kernel.h"
#include "../libc/string.h"
#include "shell.h"
#include "task.h"
#include <stdint.h>

#define BACKSPACE 14
#define ENTER 28
#define LSHIFT_IN 42
#define LSHIFT_OUT 170
#define RSHIFT 54

#define SC_MAX 57

static char key_buffer[256];
static char last_command[256];
static int shift = 0;

void kernel_main() {
    isr_install();
    irq_install();
    kprint("isr initialized\n");
    init_tasking();
    rtc_install();
    kprint("rtc initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    ckprint("version ", c_dmagenta);
    ckprint(VERSION, c_magenta);
    kprint("\n\n");
    shell_omp();
}

void shell_mod(char letter, int scancode) {

    if (scancode == LSHIFT_IN) shift = 1;
    else if (scancode == LSHIFT_OUT) shift = 0;
    else if (scancode > SC_MAX) return;

    else if (scancode == RSHIFT) {
        for (int i = 0; last_command[i] != '\0'; i++) {
            append(key_buffer, last_command[i]);
        }
        ckprint(last_command, c_blue);
    }

    else if (scancode == BACKSPACE) {
        if (strlen(key_buffer)){
            backspace(key_buffer);
            kprint_backspace();
        }
    }
    
    else if (scancode == ENTER) {
        kprint("\n");
        strcpy_s(last_command, key_buffer);
        shell_command(key_buffer);
        key_buffer[0] = '\0';
    }

    else if (letter != '?') {
        char str[2] = {letter, '\0'};
        append(key_buffer, letter);
        ckprint(str, c_blue);
    }
}

void user_input(int scancode) {
    char letter = scancode_to_char(scancode, shift);
    shell_mod(letter, scancode);
}
