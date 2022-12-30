#include <driver/mouse.h>
#include <cpu/ports.h>  
#include <cpu/isr.h>
#include <minilib.h>
#include <system.h>
#include <type.h>

/*
Mouse.inc by SANiK
License: Use as you wish, except to cause damage
*/

byte mouse_cycle=0;     //unsigned char
sbyte mouse_byte[3];    //signed char
int mouse_x=0;         //signed char
int mouse_y=0;         //signed char
int was_installed=0;
int is_bad = 0;

/*
buttons[0] = left button
buttons[1] = right button
buttons[2] = middle button
*/
bool buttons[3];

void mouse_reset();

//Mouse functions
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
                mouse_cycle=0;
                if (!is_bad) {
                    sys_warning("Bad mouse packet (but dont worry)");
                }
                break;
            }
            if (!(mouse_byte[0] & 0x8)) {
                mouse_cycle=0;
                if (!is_bad) {
                    sys_warning("Bad mouse packet (WTF IT SHOULDNT HAPPEND WHY DO YOU USE THE SCROLL WHEEL)");
                }
                is_bad = 1;
                was_installed=0;
                mouse_install();
                mouse_reset();
                break;
            }
            // now we add the coordinates (and take care of the sign)
            if (mouse_byte[0] & 0x10) {
                mouse_x += (mouse_byte[1]);
            } else {
                mouse_x += (mouse_byte[1]);
            }
            if (mouse_byte[0] & 0x20) {
                mouse_y += -(mouse_byte[2]);
            } else {
                mouse_y += -(mouse_byte[2]);
            }

            // we check if the mouse is out of the screen
            if (mouse_x < 0) {
                mouse_x = 0;
            }
            if (mouse_y < 0) {
                mouse_y = 0;
            }
            // TODO : the screen resolution
            if (mouse_x > 1024) {
                mouse_x = 1024;
            }
            if (mouse_y > 768) {
                mouse_y = 768;
            }

            mouse_cycle=0;
            buttons[0] = (mouse_byte[0] & 0x1) == 0x1;
            buttons[1] = (mouse_byte[0] & 0x2) == 0x2;
            buttons[2] = (mouse_byte[0] & 0x4) == 0x4;
            break;
    }
}

void mouse_write(byte a_write) {
    //Tell the mouse we are sending a command
    port_byte_out(0x64, 0xD4);
    //Finally write
    port_byte_out(0x60, a_write);
}

byte mouse_read() {
    //Get's response from mouse
    return port_byte_in(0x60);
}

void mouse_install() {
    if(was_installed) {
        return;
    }
    was_installed=1;
    byte _status;

    //Enable the auxiliary mouse device
    port_byte_out(0x64, 0xA8);

    //Enable the interrupts
    port_byte_out(0x64, 0x20);
    _status=(port_byte_in(0x60) | 2);
    port_byte_out(0x64, 0x60);
    port_byte_out(0x60, _status);

    //Tell the mouse to use default settings
    mouse_write(0xF6);
    mouse_read();  //Acknowledge

    //Enable the mouse
    mouse_write(0xF4);
    mouse_read();  //Acknowledge

    //Setup the mouse handler
    register_interrupt_handler(IRQ12, mouse_handler);

    mouse_reset();
    is_bad = 0;
}

// get data
int mouse_get_x() {
    return is_bad ? 0 : mouse_x;
}

int mouse_get_y() {
    return is_bad ? 0 : mouse_y;
}

bool mouse_get_button(int button) {
    return is_bad ? 0 : buttons[button];
}

// set data
void mouse_set_x(int x) {
    mouse_x = x;
}

void mouse_set_y(int y) {
    mouse_y = y;
}

void mouse_set_button(int button, bool value) {
    buttons[button] = value;
}

// reset data
void mouse_reset() {
    mouse_x = 0;
    mouse_y = 0;
    buttons[0] = 0;
    buttons[1] = 0;
    buttons[2] = 0;
}