#include "../cpu/isr.h"
#include "../drivers/screen.h"
#include "kernel.h"
#include "../libc/string.h"
#include "../libc/mem.h"
#include <stdint.h>

static char key_buffer[256];

void kernel_main() {
    isr_install();
    irq_install();

    asm("int $2");
    asm("int $3");

    rainbow_print("\n\nWelcome to profanOS!\n");
    kprint("profanOS-> ");
}

void shell_command(char *command) {
    if (strcmp(command, "END") == 0) {
        kprint("Stopping the CPU. Bye!\n");
        asm volatile("hlt");
    }
    
    else if (strcmp(command, "PAGE") == 0) {
        // Lesson 22: Code to test kmalloc, the rest is unchanged
        uint32_t phys_addr;
        uint32_t page = kmalloc(1000, 1, &phys_addr);
        char page_str[16] = "";
        hex_to_ascii(page, page_str);
        char phys_str[16] = "";
        hex_to_ascii(phys_addr, phys_str);
        kprint("Page: ");
        kprint(page_str);
        kprint(", physical address: ");
        kprint(phys_str);
    }

    else if (strcmp(command, "TEST") == 0) {
        ckprint(return_int_to_ascii(42), c_magenta);
    }

    else if (strcmp(command, "CLEAR") == 0) {
        clear_screen();
    }

    else if (strcmp(command, "") != 0) { kprint("not found"); }
    if (strcmp(command, "") * strcmp(command, "CLEAR") != 0) { kprint("\n"); }

    kprint("profanOS-> ");
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
        kprint(str);
    }
}
