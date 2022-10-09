#include <gui/front.h>
#include <ports.h>
#include <mem.h>

#define peekb(S,O)        * (unsigned char *)(16uL * (S) + (O))
#define pokeb(S,O,V)      * (unsigned char *)(16uL * (S) + (O)) = (V)
#define pokew(S,O,V)      * (unsigned short *)(16uL * (S) + (O)) = (V)
#define _vmemwr(DS,DO,S,N)  mem_copy((uint8_t *)((DS) * 16 + (DO)), S, N)

// define the ports, taken from http://files.osdev.org/mirrors/geezer/osd/graphics/modes.c
#define vga_AC_INDEX 0x3C0
#define vga_AC_WRITE 0x3C0
#define vga_AC_READ 0x3C1
#define vga_MISC_WRITE 0x3C2
#define vga_SEQ_INDEX 0x3C4
#define vga_SEQ_DATA 0x3C5
#define vga_DAC_READ_INDEX 0x3C7
#define vga_DAC_WRITE_INDEX 0x3C8
#define vga_DAC_DATA 0x3C9
#define vga_MISC_READ 0x3CC
#define vga_GC_INDEX 0x3CE
#define vga_GC_DATA 0x3CF
#define vga_CRTC_INDEX 0x3D4
#define vga_CRTC_DATA 0x3D5
#define vga_INSTAT_READ 0x3DA
#define vga_NUM_SEQ_REGS 5
#define vga_NUM_CRTC_REGS 25
#define vga_NUM_GC_REGS 9
#define vga_NUM_AC_REGS 21
#define vga_NUM_REGS (1 + vga_NUM_SEQ_REGS + vga_NUM_CRTC_REGS + vga_NUM_GC_REGS + vga_NUM_AC_REGS)

// the vga identifiers
unsigned int   vga_width;
unsigned int   vga_height;
unsigned int   vga_bpp;
unsigned char *vga_address;

// CREATE THE REGISTER ARRAY TAKEN FROM http://wiki.osdev.org/vga_Hardware
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

unsigned char g_80x25_text[] = {
    0x67, 0x03, 0x00, 0x03, 0x00, 0x02, 0x5F, 0x4F,
    0x50, 0x82, 0x55, 0x81, 0xBF, 0x1F, 0x00, 0x4F,
    0x0D, 0x0E, 0x00, 0x00, 0x00, 0x50, 0x9C, 0x0E,
    0x8F, 0x28, 0x1F, 0x96, 0xB9, 0xA3, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00
};

unsigned char g_90x60_text[] = {
    0xE7, 0x03, 0x01, 0x03, 0x00, 0x02, 0x6B, 0x59,
    0x5A, 0x82, 0x60, 0x8D, 0x0B, 0x3E, 0x00, 0x47,
    0x06, 0x07, 0x00, 0x00, 0x00, 0x00, 0xEA, 0x0C,
    0xDF, 0x2D, 0x08, 0xE8, 0x05, 0xA3, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x10, 0x0E, 0x00, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x0C, 0x00, 0x0F, 0x08, 0x00,
};

void write_registers(unsigned char *regs) {
    unsigned i;

    // write MISCELLANEOUS reg
    port_byte_out(vga_MISC_WRITE, *regs);
    regs++;
    // write SEQUENCER regs
    for (i = 0; i < vga_NUM_SEQ_REGS; i++) {
        port_byte_out(vga_SEQ_INDEX, i);
        port_byte_out(vga_SEQ_DATA, *regs);
        regs++;
    }
    // unlock CRTC registers
    port_byte_out(vga_CRTC_INDEX, 0x03);
    port_byte_out(vga_CRTC_DATA, port_byte_in(vga_CRTC_DATA) | 0x80);
    port_byte_out(vga_CRTC_INDEX, 0x11);
    port_byte_out(vga_CRTC_DATA, port_byte_in(vga_CRTC_DATA) & ~0x80);
    // make sure they remain unlocked
    regs[0x03] |= 0x80;
    regs[0x11] &= ~0x80;
    // write CRTC regs
    for (i = 0; i < vga_NUM_CRTC_REGS; i++) {
        port_byte_out(vga_CRTC_INDEX, i);
        port_byte_out(vga_CRTC_DATA, *regs);
        regs++;
    }
    // write GRAPHICS CONTROLLER regs
    for (i = 0; i < vga_NUM_GC_REGS; i++) {
        port_byte_out(vga_GC_INDEX, i);
        port_byte_out(vga_GC_DATA, *regs);
        regs++;
    }
    // write ATTRIBUTE CONTROLLER regs
    for (i = 0; i < vga_NUM_AC_REGS; i++) {
        (void) port_byte_in(vga_INSTAT_READ);
        port_byte_out(vga_AC_INDEX, i);
        port_byte_out(vga_AC_WRITE, *regs);
        regs++;
    }

    // lock 16-color palette and unblank display
    (void) port_byte_in(vga_INSTAT_READ);
    port_byte_out(vga_AC_INDEX, 0x20);
}

void vga_put_pixel(int x, int y, unsigned char color) {
    vga_address[vga_width * y + x] = color;
}

void vga_clear_screen() {
    for (unsigned int xy = 0; xy < vga_height * vga_width; xy++) {
        vga_put_pixel(xy % vga_width, xy / vga_width, 0);
    }
}

void vga_init() {
    // setup the vga struct
    vga_width   = 320;
    vga_height  = 200;
    vga_bpp     = 256;
    vga_address = (void *) 0xA0000;

    // enables the mode 13 state
    write_registers(mode_320_200_256);

    // clears the screen
    vga_clear_screen();
}

int vga_get_width() {
    return vga_width;
}

int vga_get_height() {
    return vga_height;
}

unsigned get_fb_seg() {
    unsigned seg;

    port_byte_out(vga_GC_INDEX, 6);
    seg = port_byte_in(vga_GC_DATA);
    seg >>= 2;
    seg &= 3;
    switch(seg) {
    case 0:
    case 1:
        seg = 0xA000;
        break;
    case 2:
        seg = 0xB000;
        break;
    case 3:
        seg = 0xB800;
        break;
    }
    return seg;
}

void vmemwr(unsigned dst_off, unsigned char *src, unsigned count) {
    _vmemwr(get_fb_seg(), dst_off, src, count);
}

void set_plane(unsigned p) {
    unsigned char pmask;

    p &= 3;
    pmask = 1 << p;
    // set read plane
    port_byte_out(vga_GC_INDEX, 4);
    port_byte_out(vga_GC_DATA, p);
    // set write plane
    port_byte_out(vga_SEQ_INDEX, 2);
    port_byte_out(vga_SEQ_DATA, pmask);
}

void write_font(unsigned char *buf, unsigned font_height) {
    unsigned char seq2, seq4, gc4, gc5, gc6;

    // save registers
    // set_plane() modifies GC 4 and SEQ 2, so save them as well
    port_byte_out(vga_SEQ_INDEX, 2);
    seq2 = port_byte_in(vga_SEQ_DATA);

    port_byte_out(vga_SEQ_INDEX, 4);
    seq4 = port_byte_in(vga_SEQ_DATA);
    // turn off even-odd addressing (set flat addressing)
    // assume: chain-4 addressing already off
    port_byte_out(vga_SEQ_DATA, seq4 | 0x04);

    port_byte_out(vga_GC_INDEX, 4);
    gc4 = port_byte_in(vga_GC_DATA);

    port_byte_out(vga_GC_INDEX, 5);
    gc5 = port_byte_in(vga_GC_DATA);
    // turn off even-odd addressing
    port_byte_out(vga_GC_DATA, gc5 & ~0x10);

    port_byte_out(vga_GC_INDEX, 6);
    gc6 = port_byte_in(vga_GC_DATA);
    // turn off even-odd addressing
    port_byte_out(vga_GC_DATA, gc6 & ~0x02);
    // write font to plane P4
    set_plane(2);
    // write font 0
    for (unsigned i = 0; i < 256; i++) {
        vmemwr(16384u * 0 + i * 32, buf, font_height);
        buf += font_height;
    }
    // restore registers
    port_byte_out(vga_SEQ_INDEX, 2);
    port_byte_out(vga_SEQ_DATA, seq2);
    port_byte_out(vga_SEQ_INDEX, 4);
    port_byte_out(vga_SEQ_DATA, seq4);
    port_byte_out(vga_GC_INDEX, 4);
    port_byte_out(vga_GC_DATA, gc4);
    port_byte_out(vga_GC_INDEX, 5);
    port_byte_out(vga_GC_DATA, gc5);
    port_byte_out(vga_GC_INDEX, 6);
    port_byte_out(vga_GC_DATA, gc6);
}

void vga_switch_text_mode(int hi_res) {
    if (hi_res) {
        write_registers(g_90x60_text);
        vga_width = 90;
        vga_height = 60;
        vga_bpp = 8;
    } else {
        write_registers(g_80x25_text);
        vga_width = 80;
        vga_height = 25;
        vga_bpp = 16;
    }

    if (vga_bpp >= 16) write_font(g_8x16_font, 16);
    else write_font(g_8x8_font, 8);

    // tell the BIOS what we've done, so BIOS text output works OK
    pokew(0x40, 0x4A, vga_width);           // columns on screen
    pokew(0x40, 0x4C, vga_width * vga_height * 2); // framebuffer size
    pokew(0x40, 0x50, 0);                   // cursor pos'n
    pokeb(0x40, 0x60, vga_bpp - 1);         // cursor shape
    pokeb(0x40, 0x61, vga_bpp - 2);
    pokeb(0x40, 0x84, vga_height - 1);      // rows on screen - 1
    pokeb(0x40, 0x85, vga_bpp);             // char height
    // set white-on-black attributes for all text
    for(unsigned i = 0; i < vga_width * vga_height; i++)
        pokeb(0xB800, i * 2 + 1, 7);
}
