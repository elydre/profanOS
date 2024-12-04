/*****************************************************************************\
|   === keyboard.c : 2024 ===                                                 |
|                                                                             |
|    Kernel Keyboard driver                                        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <drivers/keyboard.h>
#include <kernel/process.h>
#include <cpu/ports.h>
#include <cpu/isr.h>
#include <system.h>

#define HISTORY_SIZE 5

static int sc_history[HISTORY_SIZE];

char kb_sc_to_char(int scancode, int shift) {
    char sc_ascii_min[] = {
        '\0', '\0', '&', '~', '"', '\'','(', '-', '`', '_',
        '+', '@', ')', '=', '\0', '\0', 'a', 'z', 'e', 'r',
        't', 'y', 'u', 'i', 'o', 'p', '^', '$', '\0', '\0',
        'q', 's', 'd', 'f', 'g', 'h', 'j', 'k', 'l', 'm',
        '%', '\\','\0', '*', 'w', 'x', 'c', 'v', 'b', 'n',
        ',', ';', ':', '!', '\0', '\0', '\0', ' '
    };

    char sc_ascii_maj[] = {
        '\0', '\0', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '[', ']', '\0', '\0', 'A', 'Z', 'E', 'R',
        'T', 'Y', 'U', 'I', 'O', 'P', '{', '}', '\0', '\0',
        'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
        '<', '#', '\0', '>', 'W', 'X', 'C', 'V', 'B', 'N',
        '?', '.', '/', '+', '\0', '\0', '\0', ' '
    };

    return (shift) ? sc_ascii_maj[scancode] : sc_ascii_min[scancode];
}

int kb_get_scancode(void) {
    return (int) port_byte_in(0x60);
}

int kb_get_scfh(void) {
    // get scancode from history
    for (int i = HISTORY_SIZE - 1; i >= 0; i--) {
        if (sc_history[i] == 0) continue;
        int sc = sc_history[i];
        sc_history[i] = 0;
        return sc;
    }
    return 0;
}

static void keyboard_callback(registers_t *regs) {
    (void) regs;

    int sc = kb_get_scancode();

    if (sc == 0) return;
    if (sc == sc_history[0]) return;

    for (int i = HISTORY_SIZE - 2; i >= 0; i--) {
        sc_history[i + 1] = sc_history[i];
    }
    sc_history[0] = sc;

    if (sc_history[0] == 60) {   // F2
        int pid = process_create(kernel_exit_current, 0, 0, NULL);
        if (pid > 0) process_wakeup(pid, 0);
    }
}

int keyboard_init(void) {
    register_interrupt_handler(IRQ1, keyboard_callback);
    for (int i = 0; i < HISTORY_SIZE; i++) {
        sc_history[i] = 0;
    }

    return 0;
}
