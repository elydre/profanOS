#include <ports.h>

// define the ports, taken from http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
#define VGA_AC_INDEX 0x3C0
#define VGA_AC_WRITE 0x3C0
#define VGA_AC_READ 0x3C1
#define VGA_MISC_WRITE 0x3C2
#define VGA_SEQ_INDEX 0x3C4
#define VGA_SEQ_DATA 0x3C5
#define VGA_DAC_READ_INDEX 0x3C7
#define VGA_DAC_WRITE_INDEX 0x3C8
#define VGA_DAC_DATA 0x3C9
#define VGA_MISC_READ 0x3CC
#define VGA_GC_INDEX 0x3CE
#define VGA_GC_DATA 0x3CF
#define VGA_CRTC_INDEX 0x3D4
#define VGA_CRTC_DATA 0x3D5
#define VGA_INSTAT_READ 0x3DA
#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

// the vga identifiers
unsigned int   VGA_width;
unsigned int   VGA_height;
unsigned int   VGA_bpp;
unsigned char *VGA_address;

// CREATE THE REGISTER ARRAY TAKEN FROM http://wiki.osdev.org/VGA_Hardware
unsigned char mode_320_200_256[] = {
    0x63, 0x03, 0x01, 0x0F, 0x00, 0x0E, 0x5F, 0x4F,
    0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E,
    0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};


void write_registers(unsigned char *regs) {
    unsigned i;

    // write MISCELLANEOUS reg
    port_byte_out(VGA_MISC_WRITE, *regs);
    regs++;
    // write SEQUENCER regs
    for (i = 0; i < VGA_NUM_SEQ_REGS; i++) {
        port_byte_out(VGA_SEQ_INDEX, i);
        port_byte_out(VGA_SEQ_DATA, *regs);
        regs++;
    }
    // unlock CRTC registers
    port_byte_out(VGA_CRTC_INDEX, 0x03);
    port_byte_out(VGA_CRTC_DATA, port_byte_in(VGA_CRTC_DATA) | 0x80);
    port_byte_out(VGA_CRTC_INDEX, 0x11);
    port_byte_out(VGA_CRTC_DATA, port_byte_in(VGA_CRTC_DATA) & ~0x80);
    // make sure they remain unlocked
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;
    // write CRTC regs
    for (i = 0; i < VGA_NUM_CRTC_REGS; i++) {
        port_byte_out(VGA_CRTC_INDEX, i);
        port_byte_out(VGA_CRTC_DATA, *regs);
        regs++;
    }
    // write GRAPHICS CONTROLLER regs
    for (i = 0; i < VGA_NUM_GC_REGS; i++) {
        port_byte_out(VGA_GC_INDEX, i);
        port_byte_out(VGA_GC_DATA, *regs);
        regs++;
    }
    // write ATTRIBUTE CONTROLLER regs
    for (i = 0; i < VGA_NUM_AC_REGS; i++) {
        (void) port_byte_in(VGA_INSTAT_READ);
        port_byte_out(VGA_AC_INDEX, i);
        port_byte_out(VGA_AC_WRITE, *regs);
        regs++;
    }

    // lock 16-color palette and unblank display
    (void) port_byte_in(VGA_INSTAT_READ);
    port_byte_out(VGA_AC_INDEX, 0x20);
}

void vga_put_pixel(int x, int y, unsigned char color) {
    VGA_address[VGA_width * y + x] = color;
}

void VGA_clear_screen() {
    for (unsigned int xy = 0; xy < VGA_height * VGA_width; xy++) {
        vga_put_pixel(xy % VGA_width, xy / VGA_width, 0);
    }
}

void vga_init() {
    // setup the vga struct
    VGA_width   = 320;
    VGA_height  = 200;
    VGA_bpp     = 256;
    VGA_address = (void *) 0xA0000;

    // enables the mode 13 state
    write_registers(mode_320_200_256);

    // clears the screen
    VGA_clear_screen();
}

int vga_get_width() {
    return VGA_width;
}

int vga_get_height() {
    return VGA_height;
}
