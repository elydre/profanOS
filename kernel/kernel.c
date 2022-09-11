#include <driver/keyboard.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <cpu/isr.h>
#include <driver/screen.h>
#include <string.h>
#include "kernel.h"
#include <iolib.h>
#include <time.h>
#include <task.h>

#include <stdint.h>

#define SC_MAX 57

#define LSHIFT_IN 42
#define LSHIFT_OUT 170
#define RSHIFT_IN 54
#define RSHIFT_OUT 182

#define ENTER 28
#define BACKSPACE 14
#define LAST_CMD 72


static char key_buffer[256];
static char last_command[256];
static int shift = 0;
static int boot_time;

void kernel_main() {
    isr_install();
    irq_install();
    kprint("ISR initialized\n");
    init_tasking();
    kprint("Tasking initialized\n");
    rtc_install();
    kprint("RTC initialized\n");

    boot_time = gen_unix_time();

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$Cversion $4%s\n\n", VERSION);
    shell_omp();
}

void shell_mod(char letter, int scancode) {
    char str[2] = {letter, '\0'};

    if (scancode == LSHIFT_IN || scancode == RSHIFT_IN) shift = 1;
    else if (scancode == LSHIFT_OUT || scancode == RSHIFT_OUT) shift = 0;

    else if (scancode == LAST_CMD) {
        for (int i = 0; last_command[i] != '\0'; i++) {
            if (strlen(key_buffer) > 250) break;
            append(key_buffer, last_command[i]);
            str[0] = last_command[i];
            ckprint(str, c_blue);
        }
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

    else if (scancode > SC_MAX) return;
    else if (letter != '?') {
        if (strlen(key_buffer) > 250) return;
        append(key_buffer, letter);
        ckprint(str, c_blue);
    }
}

void user_input(int scancode) {
    char letter = scancode_to_char(scancode, shift);
    shell_mod(letter, scancode);
}

int get_boot_time() {
    return boot_time;
}
