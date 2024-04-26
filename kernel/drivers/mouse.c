/*****************************************************************************\
|   === mouse.c : 2024 ===                                                    |
|                                                                             |
|    Kernel Mouse driver                                           .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <drivers/mouse.h>
#include <cpu/ports.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <minilib.h>
#include <system.h>
#include <ktype.h>

// mouse.inc by SANiK
// license: Use as you wish, except to cause damage

int8_t mouse_byte[3];

uint8_t mouse_cycle = 0;
int was_installed = 0;
int is_bad = 0;

int mouse_x;
int mouse_y;

/*******************************
 * buttons[0] = left button   *
 * buttons[1] = right button  *
 * buttons[2] = middle button *
*******************************/

uint8_t buttons[3];

void mouse_reset(void);

// mouse functions
void mouse_handler(registers_t *a_r) { // (not used but just there)
    UNUSED(a_r);
    switch(mouse_cycle) {
        case 0:
            mouse_byte[0] = port_byte_in(0x60);
            mouse_cycle++;
            break;
        case 1:
            mouse_byte[1] = port_byte_in(0x60);
            mouse_cycle++;
            break;
        case 2:
            mouse_byte[2]=port_byte_in(0x60);

            // if those are set, it's a bad packet
            if ((mouse_byte[0] & 0x80) || (mouse_byte[0] & 0x40)) {
                mouse_cycle = 0;
                break;
            }
            if (!(mouse_byte[0] & 0x8)) {
                mouse_cycle=0;
                is_bad = 1;
                was_installed = 0;
                // TODO : save the old mouse position so we can restore it
                mouse_init();
                mouse_reset();
                break;
            }
            // now we add the coordinates (and take care of the sign)
            if (mouse_byte[0] & 0x10) {
                mouse_x += mouse_byte[1];
            } else {
                mouse_x += mouse_byte[1];
            }
            if (mouse_byte[0] & 0x20) {
                mouse_y -= mouse_byte[2];
            } else {
                mouse_y -= mouse_byte[2];
            }
            // we check if the mouse is out of the screen
            if (mouse_x < 0) {
                mouse_x = 0;
            }
            if (mouse_y < 0) {
                mouse_y = 0;
            }
            if (mouse_x > (int) vesa_get_width()) {
                mouse_x = vesa_get_width();
            }
            if (mouse_y > (int) vesa_get_height()) {
                mouse_y = vesa_get_height();
            }

            mouse_cycle=0;
            buttons[0] = (mouse_byte[0] & 0x1) == 0x1;
            buttons[1] = (mouse_byte[0] & 0x2) == 0x2;
            buttons[2] = (mouse_byte[0] & 0x4) == 0x4;

            is_bad = 0;
            break;
    }
}

void mouse_write(uint8_t a_write) {
    // tell the mouse we are sending a command
    port_byte_out(0x64, 0xD4);
    // finally write
    port_byte_out(0x60, a_write);
}

uint8_t mouse_read(void) {
    // get's response from mouse
    return port_byte_in(0x60);
}

int mouse_init(void) {
    if(was_installed) {
        return 1;
    }
    was_installed = 1;

    uint8_t status;

    // enable the auxiliary mouse device
    port_byte_out(0x64, 0xA8);

    // enable the interrupts
    port_byte_out(0x64, 0x20);
    status = (port_byte_in(0x60) | 2);
    port_byte_out(0x64, 0x60);
    port_byte_out(0x60, status);

    // tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();  // acknowledge

    // enable the mouse
    mouse_write(0xF4);
    mouse_read();  // acknowledge

    // setup the mouse handler
    register_interrupt_handler(IRQ12, mouse_handler);

    mouse_reset();
    return 0;
}

// reset data
void mouse_reset(void) {
    mouse_x = 0;
    mouse_y = 0;
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = 0;
}

// get/set mouse data
int mouse_call(int thing, int val) {
    switch(thing) {
        case 0:
            return is_bad ? 0 : mouse_x;
        case 1:
            return is_bad ? 0 : mouse_y;
        case 2:
            return is_bad ? 0 : buttons[val];
        case 3:
            mouse_x = val;
            return 0;
        case 4:
            mouse_y = val;
            return 0;
        case 5:
            mouse_reset();
            return 0;
        default:
            return 0;
    }
}
