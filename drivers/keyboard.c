#include "../kernel/kernel.h"
#include <function.h>
#include <keyboard.h>
#include <screen.h>
#include <string.h>
#include <ports.h>
#include <isr.h>

#include <stdint.h>


char scancode_to_char(int scancode, int shift) {
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

int get_last_scancode() {
    return (int) port_byte_in(0x60);
}

static void keyboard_callback(registers_t *regs) {
    // The PIC leaves us the scancode in port 0x60
    uint8_t scancode = port_byte_in(0x60);
    
    user_input((int)scancode);
    UNUSED(regs);
}

void init_keyboard() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
}
