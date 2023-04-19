#include <driver/mouse.h>
#include <cpu/ports.h>
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

uint8_t g_commandRunning, g_resetStages, g_mouseDeviceID = 0, g_mouseCycle;
uint8_t g_mouseAvailable = 0, g_mouseInitted = 0, g_ps2MouseAvail = 0, g_ps2DisableMovement = 0;

int g_mouseX, g_mouseY;
uint8_t g_mouseButtons[3];

mouse_packet_t g_currentPacket;
uint8_t g_discardPacket;

uint8_t IsMouseAvailable() {
    return g_mouseAvailable && g_mouseInitted;
}

// mouse comms //

void MouseWait (uint8_t type) {
    if (!g_mouseAvailable) return;
    uint32_t _timeout = 1000000;

    if (type == 0) {
        while (_timeout--) {
            if (port_byte_in (0x64) & 1) return;
            asm ("pause");
        }
    } else {
        while (_timeout--) {
            if (!(port_byte_in (0x64) & 2)) return;
            asm ("pause");
        }
    }
    g_mouseAvailable = 0;
}

void MouseWrite (uint8_t write) {
    if (!g_mouseAvailable) return;
    MouseWait (1);
    port_byte_out(0x64, 0xD4);
    MouseWait (1);
    port_byte_out(0x60, write);
}

uint8_t MouseRead() {
    if (!g_mouseAvailable)
        return 0xFF;
    MouseWait (0);
    return port_byte_in (0x60);
}


static const int g_sampleRateValues[] = {10,20,40,60,80,100,200};
int g_mouseSpeedMultiplier = 2, g_mouseSampRate = 6;

int GetMouseSpeedMultiplier() {
    return g_mouseSpeedMultiplier;
}

int GetMouseSampRateMax() {
    return ARYLEN(g_sampleRateValues);
}

void SetMouseSpeedMultiplier(int spd) {
    spd &= 0b11;
    g_mouseSpeedMultiplier = spd;

    if (g_ps2MouseAvail)
    {
        g_commandRunning = COMMAND_SETRESOLUTION;
        MouseWrite(0xE8);
    }
}

void SetMouseSampleRate(int spd) {
    if (spd < 0) spd = 0;
    if (spd >= GetMouseSampRateMax()) spd = ARYLEN(g_sampleRateValues);
    g_mouseSampRate = spd;

    // send Set Sample Rate command
    if (g_ps2MouseAvail)
    {
        g_commandRunning = COMMAND_SETSAMPRATE;
        MouseWrite(0xF3);
    }
}

void OnUpdateMouse (uint8_t flags, int8_t x, int8_t y, int8_t z) {
    (void) z;
    g_mouseX += x;
    g_mouseY -= y;
    if (g_mouseX < 0) g_mouseX = 0;
    if (g_mouseY < 0) g_mouseY = 0;
    if (g_mouseX >= 1024) g_mouseX = 1023;
    if (g_mouseY >= 768) g_mouseY = 767;

    g_mouseButtons[0] = (flags & 0b1) ? 1 : 0;
    g_mouseButtons[1] = (flags & 0b10) ? 1 : 0;
    g_mouseButtons[2] = (flags & 0b100) ? 1 : 0;
}

void IrqMouse() {
    //acknowledge interrupt
    port_byte_out(0x20, 0x20);
    port_byte_out(0xA0, 0x20); // irq 12!!!

    uint8_t b = MouseRead();
    if (g_commandRunning) {
        switch (g_commandRunning) {
            case COMMAND_RESET: {
                if (b == 0xFA) {
                    //we are on good terms, return.
                    if (g_resetStages <= 3) {
                        g_resetStages++;
                        return;
                    }
                }
                else if (b == 0xAA && g_resetStages <= 1) {
                    g_resetStages = 2;
                    return;
                }
                else if (g_resetStages == 2) {
                    g_resetStages = 3; // this is the mouse ID
                    g_mouseDeviceID = b;
                    return;
                }
                else if (g_resetStages == 3) {
                    // extra data that we do not care about
                    g_resetStages = 4;
                    g_commandRunning = COMMAND_NONE;
                }
                break;
            }
            case COMMAND_SETDEFAULT: {
                // NanoShell V2 ignores this. Why?
                g_commandRunning = COMMAND_NONE;
                break;
            }
            case COMMAND_SETREPORT: {
                // NanoShell V2 ignores this. Why?
                g_commandRunning = COMMAND_NONE;
                break;
            }
            case COMMAND_SETSAMPRATE: {
                if (b == 0xFA) {
                    // acknowledged.
                    g_commandRunning = COMMAND_SETSAMPRATE1;
                    MouseWrite (g_sampleRateValues[g_mouseSampRate]);
                }
                else g_commandRunning = COMMAND_NONE;
                break;
            }
            case COMMAND_SETRESOLUTION: {
                if (b == 0xFA) {
                    // acknowledged.
                    g_commandRunning = COMMAND_SETRESOLUTION1;
                    MouseWrite (g_mouseSpeedMultiplier);
                }
                else
                    g_commandRunning = COMMAND_NONE;
                break;
            }
            case COMMAND_SETSAMPRATE1:
            case COMMAND_SETRESOLUTION1: {
                g_commandRunning = COMMAND_NONE;
                break;
            }
            case COMMAND_GETDEVID: {
                // What if the mouse ID was indeed 0xfa? Probably never the case
                if (b != 0xFA) g_mouseDeviceID = b; 
                g_commandRunning = COMMAND_NONE;
                break;
            }
        }
    } else {
        switch (g_mouseCycle) {
            case 0:
                g_currentPacket.flags = b;
                g_mouseCycle++;
                g_discardPacket = 0;
            
                if (g_currentPacket.flags & (1 << 6) || g_currentPacket.flags & (1 << 7))
                g_discardPacket = 1;
            
                if (!(g_currentPacket.flags & (1 << 3))) {
                    // WAIT UNTIL WE GET A 0x8, THEN PROCEED!!!!!!
                    // This is a hack, and should not be kept.
                    g_mouseCycle = 0;
                }
                break;
            case 1:
                g_currentPacket.xMov = b;
                g_mouseCycle++;
                break;
            case 2:
                g_currentPacket.yMov = b;
                if (g_mouseDeviceID == 0)
                {
                    // some mice do not send scroll data too
                    g_mouseCycle = 0;
                    g_commandRunning = COMMAND_NONE;
                    if (g_discardPacket) {
                        g_discardPacket = 0;
                        return;
                    }
                    if (g_ps2DisableMovement) {
                        g_currentPacket.xMov = 0;
                        g_currentPacket.yMov = 0;
                    }
                    OnUpdateMouse (g_currentPacket.flags, g_currentPacket.xMov, g_currentPacket.yMov, 0);
                }
                else g_mouseCycle++;
                break;
            case 3:
                g_currentPacket.zMov = b;
                g_mouseCycle = 0;
                g_commandRunning = COMMAND_NONE;
                if (g_discardPacket)
                {
                    g_discardPacket = 0;
                    return;
                }
                if (g_ps2DisableMovement)
                {
                    g_currentPacket.xMov = 0;
                    g_currentPacket.yMov = 0;
                }
                OnUpdateMouse (g_currentPacket.flags, g_currentPacket.xMov, g_currentPacket.yMov, g_currentPacket.zMov);
                break;
        }
    }
}

int mouse_init() {
    register_interrupt_handler(IRQ12, IrqMouse);
    g_mouseAvailable = 1;

    // return; // don't have it for now
    uint8_t _status;

    // Enable the auxiliary mouse device
    MouseWait (1);
    if (!g_mouseAvailable) return 1;
    port_byte_out(0x64, 0xA8);

    // Enable the interrupts
    MouseWait (1);
    if (!g_mouseAvailable) return 1;
    port_byte_out(0x64, 0x20);

    uint8_t b = port_byte_in (0x60);

    // HACK!!! Some PCs (my HP laptop for instance) will actually return a
    // bitflipped config, so bits 7 and 3 are set.  Just flip the whole byte
    // so everything is in order.
    if ((b & 0x80) && (b & 0x08)) b = ~b;
    // if (b & 0x88) b = ~b;
    _status = (b | 2);

    MouseWait (1);
    if (!g_mouseAvailable) return 1;
    port_byte_out(0x64, 0x60);
    MouseWait (1);
    if (!g_mouseAvailable) return 1;
    port_byte_out(0x60, _status);

    //reset mouse
    g_commandRunning = COMMAND_RESET;
    MouseWrite (255);

    for (int i = 0; i < 20; i++) {
        port_byte_out(0x80, 0x00); // Wait a few seconds to make sure all the interrupts went through.
    }

    g_commandRunning = COMMAND_NONE;

    // halt for 3 bytes, because we're supposed to get them
    g_resetStages = 4;

    // tell the mouse to use default settings
    g_commandRunning = COMMAND_SETDEFAULT;
    MouseWrite (0xF6);

    g_commandRunning = COMMAND_SETREPORT;
    MouseWrite (0xF4);

    g_commandRunning = COMMAND_GETDEVID;
    MouseWrite (0xF2);

    SetMouseSampleRate(3);

    while (port_byte_in(0x64) & 2)
        port_byte_in (0x60);

    if (g_mouseAvailable) {
        g_mouseInitted = 1;
        g_ps2MouseAvail = 1;
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
            return g_mouseButtons[val];
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
