#include "keyboard.h"
#include "../cpu/ports.h"
#include "../cpu/isr.h"
#include "screen.h"
#include "../libc/string.h"
#include "../libc/function.h"
#include "../kernel/kernel.h"
#include <stdint.h>

#define SC_MAX 57


char scancode_to_char(int scancode) {
    char sc_ascii[] = {
        '?', '?', '1', '2', '3', '4', '5', '6', '7', '8',
        '9', '0', '-', '=', '?', '?', 'A', 'Z', 'E', 'R',
        'T', 'Y', 'U', 'I', 'O', 'P', '[', ']', '?', '?',
        'Q', 'S', 'D', 'F', 'G', 'H', 'J', 'K', 'L', 'M',
        '\'','`', '?', '>', 'W', 'X', 'C', 'V', 'B', 'N',
        '!', ',', '.', '/', '?', '?', '?', ' '
    };

    return sc_ascii[scancode];
}

void scancode_to_name(int scancode, char name[]) {
    char *sc_name[] = {
        "ERROR", "Esc", "1", "2", "3", "4", "5", "6", "7",
        "8", "9", "0", "-", "=", "Backspace", "Tab", "A",
        "Z", "E", "R", "T", "Y", "U", "I", "O", "P", "[",
        "]", "Enter", "Lctrl", "Q", "S", "D", "F", "G", "H",
        "J", "K", "L", "M", "'", "`", "LShift", ">", "W", "X",
        "C", "V", "B", "N", "!", ",", ".", "/", "RShift",
        "Keypad *", "LAlt", "Spacebar"
    };

    strcpy(name, sc_name[scancode]);
}

static void keyboard_callback(registers_t *regs) {
    /* The PIC leaves us the scancode in port 0x60 */
    uint8_t scancode = port_byte_in(0x60);
    
    if (scancode > SC_MAX) return;
    user_input((int)scancode);
    UNUSED(regs);
}

void init_keyboard() {
   register_interrupt_handler(IRQ1, keyboard_callback); 
}
