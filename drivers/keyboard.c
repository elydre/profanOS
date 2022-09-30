#include <driver/keyboard.h>
#include <function.h>
#include <cpu/isr.h>
#include <ports.h>

#include <stdint.h>


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

static void keyboard_callback(registers_t *regs) {
    UNUSED(regs);
    /* This function is actually unused,
     * remplaced by the input() function
     * but it can be used for ^C exit !
    */
}

void keyboard_init() {
    register_interrupt_handler(IRQ1, keyboard_callback); 
}
