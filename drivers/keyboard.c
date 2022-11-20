#include <driver/keyboard.h>
#include <gui/gnrtx.h>
#include <libc/task.h>
#include <cpu/ports.h>
#include <function.h>
#include <cpu/isr.h>


#define HISTORY_SIZE 4

static int sc_history[HISTORY_SIZE];

char kb_scancode_to_char(int scancode, int shift) {
    char sc_ascii_min[] = {
        '?', '?', '&', '~', '"', '\'','(', '-', '`', '_',
        '+', '@', ')', '=', '?', '?', 'a', 'z', 'e', 'r',
        't', 'y', 'u', 'i', 'o', 'p', '^', '$', '?', '?',
        'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm',
        '%', '*', '?', '*', 'w', 'x', 'c', 'v', 'b', 'n',
        ',', ';', ':', '!', '?', '?', '?', ' '
    };

    char sc_ascii_maj[] = {
        '?', '?', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '[', ']', '?', '?', 'A', 'Z', 'E', 'R',
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '?', '?',
        'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
        '<', '#', '?', '>', 'W', 'X', 'C', 'V', 'B', 'N',
        '?', '.', '/', '+', '?', '?', '?', ' '
    };

    return (shift) ? sc_ascii_maj[scancode] : sc_ascii_min[scancode];
}

int kb_get_scancode() {
    return (int) port_byte_in(0x60);
}

int kb_get_scfh() {
    // get scancode from history
    for (int i = HISTORY_SIZE - 1; i >= 0; i--) {
        if (sc_history[i] == 0) continue;
        int sc = sc_history[i];
        sc_history[i] = 0;
        return sc;
    }
    return 0;
}

void kb_reset_history() {
    for (int i = 0; i < HISTORY_SIZE; i++) {
        sc_history[i] = 0;
    }
}

static void keyboard_callback(registers_t *regs) {
    UNUSED(regs);

    for (int i = 0; i < HISTORY_SIZE - 1; i++)
        sc_history[i + 1] = sc_history[i];
    sc_history[0] = kb_get_scancode();

    if (sc_history[0] == 59 && task_get_current_pid()) {
        clear_screen();
        task_switch(0);
    }
}

void keyboard_init() {
    register_interrupt_handler(IRQ1, keyboard_callback);
    kb_reset_history();
}

