#include <driver/mouse.h>
#include <cpu/ports.h>
#include <gui/vesa.h>
#include <minilib.h>

enum {
    COMMAND_NONE = 0,
    COMMAND_RESET,
    COMMAND_SETDEFAULT,
    COMMAND_SETREPORT,
    COMMAND_GETDEVID,
    COMMAND_READDATA,
    COMMAND_SETSAMPRATE, COMMAND_SETSAMPRATE1,
    COMMAND_SETRESOLUTION, COMMAND_SETRESOLUTION1
};

uint8_t g_command_running, g_reset_stages, g_mouse_device_ID = 0, g_mouse_cycle;
uint8_t g_mouse_available = 0, g_mouse_initted = 0;

int g_mouseX, g_mouseY;
uint8_t g_mouse_buttons[3];

mouse_packet_t g_currentPacket;
uint8_t g_discardPacket;

uint8_t mouse_is_available() {
    return g_mouse_available && g_mouse_initted;
}

void mouse_wait (uint8_t type) {
    if (!g_mouse_available) return;
    uint32_t timeout = 1000000;

    if (type == 0) {
        while (timeout--) {
            if (port_byte_in (0x64) & 1) return;
            asm ("pause");
        }
    } else {
        while (timeout--) {
            if (!(port_byte_in (0x64) & 2)) return;
            asm ("pause");
        }
    }
    g_mouse_available = 0;

}

void mouse_write(uint8_t write) {
    if (!g_mouse_available) return;
    mouse_wait(1);
    port_byte_out(0x64, 0xD4);
    mouse_wait(1);
    port_byte_out(0x60, write);
}

uint8_t mouse_read() {
    if (!g_mouse_available)
        return 0xFF;
    mouse_wait(0);
    return port_byte_in(0x60);
}


static const int g_sample_rate_values[] = {10, 20, 40, 60, 80, 100, 200};
int g_mouse_speed_multiplier = 2, g_mouse_samp_rate = 6;

int mouse_get_speed_multiplier() {
    return g_mouse_speed_multiplier;
}

void mouse_set_speed_multiplier(int spd) {
    spd &= 0b11;
    g_mouse_speed_multiplier = spd;

    if (g_mouse_available) {
        g_command_running = COMMAND_SETRESOLUTION;
        mouse_write(0xE8);
    }
}

int mouse_get_samp_rate_max() {
    return ARYLEN(g_sample_rate_values);
}

void mouse_set_sample_rate(int spd) {
    if (spd < 0) spd = 0;
    if (spd >= mouse_get_samp_rate_max()) spd = ARYLEN(g_sample_rate_values);
    g_mouse_samp_rate = spd;

    // send set sample rate command
    if (g_mouse_available) {
        g_command_running = COMMAND_SETSAMPRATE;
        mouse_write(0xF3);
    }
}

void update_mouse (uint8_t flags, int8_t x, int8_t y, int8_t z) {
    (void) z;
    g_mouseX += x;
    g_mouseY -= y;
    if (g_mouseX < 0) g_mouseX = 0;
    if (g_mouseY < 0) g_mouseY = 0;

    if (g_mouseX >= (int) vesa_get_width())
        g_mouseX = vesa_get_width() - 1;

    if (g_mouseY >= (int) vesa_get_height())
        g_mouseY = vesa_get_height() - 1;

    g_mouse_buttons[0] = (flags & 0b1) ? 1 : 0;
    g_mouse_buttons[1] = (flags & 0b10) ? 1 : 0;
    g_mouse_buttons[2] = (flags & 0b100) ? 1 : 0;
}

void irq_mouse() {
    // acknowledge interrupt
    port_byte_out(0x20, 0x20);
    port_byte_out(0xA0, 0x20); // irq 12

    uint8_t b = mouse_read();
    if (g_command_running) {
        switch (g_command_running) {
            case COMMAND_RESET: {
                if (b == 0xFA) {
                    // we are on good terms, return.
                    if (g_reset_stages <= 3) {
                        g_reset_stages++;
                        return;
                    }
                }
                else if (b == 0xAA && g_reset_stages <= 1) {
                    g_reset_stages = 2;
                    return;
                }
                else if (g_reset_stages == 2) {
                    g_reset_stages = 3; // this is the mouse ID
                    g_mouse_device_ID = b;
                    return;
                }
                else if (g_reset_stages == 3) {
                    // extra data that we do not care about
                    g_reset_stages = 4;
                    g_command_running = COMMAND_NONE;
                }
                break;
            }
            case COMMAND_SETDEFAULT: {
                g_command_running = COMMAND_NONE;
                break;
            }
            case COMMAND_SETREPORT: {
                g_command_running = COMMAND_NONE;
                break;
            }
            case COMMAND_SETSAMPRATE: {
                if (b == 0xFA) {
                    // acknowledged.
                    g_command_running = COMMAND_SETSAMPRATE1;
                    mouse_write(g_sample_rate_values[g_mouse_samp_rate]);
                }
                else g_command_running = COMMAND_NONE;
                break;
            }
            case COMMAND_SETRESOLUTION: {
                if (b == 0xFA) {
                    // acknowledged.
                    g_command_running = COMMAND_SETRESOLUTION1;
                    mouse_write(g_mouse_speed_multiplier);
                }
                else
                    g_command_running = COMMAND_NONE;
                break;
            }
            case COMMAND_SETSAMPRATE1:
            case COMMAND_SETRESOLUTION1: {
                g_command_running = COMMAND_NONE;
                break;
            }
            case COMMAND_GETDEVID: {
                // what if the mouse ID was indeed 0xfa? probably never the case
                if (b != 0xFA) g_mouse_device_ID = b;
                g_command_running = COMMAND_NONE;
                break;
            }
        }
    } else {
        switch (g_mouse_cycle) {
            case 0:
                g_currentPacket.flags = b;
                g_mouse_cycle++;
                g_discardPacket = 0;

                if (g_currentPacket.flags & (1 << 6) || g_currentPacket.flags & (1 << 7))
                g_discardPacket = 1;

                if (!(g_currentPacket.flags & (1 << 3))) {
                    g_mouse_cycle = 0;
                }
                break;
            case 1:
                g_currentPacket.x_mov = b;
                g_mouse_cycle++;
                break;
            case 2:
                g_currentPacket.y_mov = b;
                if (g_mouse_device_ID == 0) {
                    // some mice do not send scroll data too
                    g_mouse_cycle = 0;
                    g_command_running = COMMAND_NONE;
                    if (g_discardPacket) {
                        g_discardPacket = 0;
                        return;
                    }
                    update_mouse(g_currentPacket.flags, g_currentPacket.x_mov, g_currentPacket.y_mov, 0);
                }
                else g_mouse_cycle++;
                break;
            case 3:
                g_currentPacket.z_mov = b;
                g_mouse_cycle = 0;
                g_command_running = COMMAND_NONE;
                if (g_discardPacket) {
                    g_discardPacket = 0;
                    return;
                }
                update_mouse(g_currentPacket.flags, g_currentPacket.x_mov, g_currentPacket.y_mov, g_currentPacket.z_mov);
                break;
        }
    }
}

int mouse_init() {
    g_mouseX = vesa_get_width() / 3;
    g_mouseY = vesa_get_height() / 3;

    register_interrupt_handler(IRQ12, irq_mouse);
    g_mouse_available = 1;

    uint8_t status;

    // Enable the auxiliary mouse device
    mouse_wait(1);
    if (!g_mouse_available) return 1;
    port_byte_out(0x64, 0xA8);

    // Enable the interrupts
    mouse_wait(1);
    if (!g_mouse_available) return 1;
    port_byte_out(0x64, 0x20);

    uint8_t b = port_byte_in(0x60);

    // HACK!!! Some PCs will actually return a bitflipped config, so bits
    // 7 and 3 are set. Just flip the whole byte so everything is in order.

    if ((b & 0x80) && (b & 0x08)) b = ~b;
    status = (b | 2);

    mouse_wait(1);
    if (!g_mouse_available) return 1;
    port_byte_out(0x64, 0x60);
    mouse_wait(1);
    if (!g_mouse_available) return 1;
    port_byte_out(0x60, status);

    // reset mouse
    g_command_running = COMMAND_RESET;
    mouse_write(255);

    for (int i = 0; i < 20; i++) {
        port_byte_out(0x80, 0x00); // Wait a few seconds to make sure all the interrupts went through.
    }

    g_command_running = COMMAND_NONE;

    // halt for 3 bytes, because we're supposed to get them
    g_reset_stages = 4;

    // tell the mouse to use default settings
    g_command_running = COMMAND_SETDEFAULT;
    mouse_write(0xF6);

    g_command_running = COMMAND_SETREPORT;
    mouse_write(0xF4);

    g_command_running = COMMAND_GETDEVID;
    mouse_write(0xF2);

    mouse_set_sample_rate(3);

    while (port_byte_in(0x64) & 2)
        port_byte_in(0x60);

    if (g_mouse_available) {
        g_mouse_initted = 1;
        g_mouse_available = 1;
        return 0;
    } else {
        return 1;
    }
}

int mouse_call(int thing, int val) {
    switch(thing) {
        case 0:
            return g_mouseX;
        case 1:
            return g_mouseY;
        case 2:
            return g_mouse_buttons[val];
        case 3:
            g_mouseX = val;
            return 0;
        case 4:
            g_mouseY = val;
            return 0;
        default:
            return 0;
    }
}
