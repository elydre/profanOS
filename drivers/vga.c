#include <driver/screen.h>
#include <gui/front.h>
#include <function.h>
#include <gui/vga.h>
#include <ports.h>
#include <mem.h>

#define peekb(S,O)        * (unsigned char *)(16uL * (S) + (O))
#define pokeb(S,O,V)      * (unsigned char *)(16uL * (S) + (O)) = (V)
#define pokew(S,O,V)      * (unsigned short *)(16uL * (S) + (O)) = (V)
#define _vmemwr(DS,DO,S,N)  mem_copy((uint8_t *)((DS) * 16 + (DO)), S, N)

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
#define VGA_CRTC_INDEX 0x3D4    // 0x3B4
#define VGA_CRTC_DATA 0x3D5     // 0x3B5
#define VGA_INSTAT_READ 0x3DA
#define VGA_NUM_SEQ_REGS 5
#define VGA_NUM_CRTC_REGS 25
#define VGA_NUM_GC_REGS 9
#define VGA_NUM_AC_REGS 21
#define VGA_NUM_REGS (1 + VGA_NUM_SEQ_REGS + VGA_NUM_CRTC_REGS + VGA_NUM_GC_REGS + VGA_NUM_AC_REGS)

#define MEM_CPY_BASE 0xA0000
#define MEM_CPY_SIZE 0x4000

// the vga identifiers
unsigned int vga_width;     // width of the screen
unsigned int vga_height;    // height of the screen
unsigned int vga_bpp;       // bits per pixel
unsigned int vga_note;      // mode signature
unsigned int * framebuffer_segment_copy;

// CREATE THE REGISTER ARRAY TAKEN FROM http://wiki.osdev.org/vga_Hardware
unsigned char g_320x200x256[] = {
    0x63, 0x03, 0x01, 0x0F, 0x00, 0x0E, 0x5F, 0x4F,
    0x50, 0x82, 0x54, 0x80, 0xBF, 0x1F, 0x00, 0x41,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x9C, 0x0E,
    0x8F, 0x28, 0x40, 0x96, 0xB9, 0xA3, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x00, 0x40, 0x05, 0x0F, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06, 0x07,
    0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, 0x0F,
    0x41, 0x00, 0x0F, 0x00, 0x00
};

unsigned char g_640x480x16[] = {
    0xE3, 0x03, 0x01, 0x08, 0x00, 0x06, 0x5F, 0x4F,
    0x50, 0x82, 0x54, 0x80, 0x0B, 0x3E, 0x00, 0x40,
    0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xEA, 0x0C,
    0xDF, 0x28, 0x00, 0xE7, 0x04, 0xE3, 0xFF, 0x00,
    0x00, 0x00, 0x00, 0x03, 0x00, 0x05, 0x0F, 0xFF,
    0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x14, 0x07,
    0x38, 0x39, 0x3A, 0x3B, 0x3C, 0x3D, 0x3E, 0x3F,
    0x01, 0x00, 0x0F, 0x00, 0x00
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

/***************************
 * VGA INTERNAL FUNCTIONS *
****************************/

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

unsigned get_fb_seg() {
    unsigned seg;

    port_byte_out(VGA_GC_INDEX, 6);
    seg = port_byte_in(VGA_GC_DATA);
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

static void vpokeb(unsigned off, unsigned val) {
    pokeb(get_fb_seg(), off, val);
}

static unsigned vpeekb(unsigned off) {
    return peekb(get_fb_seg(), off);
}

void mcp(unsigned int *source, unsigned int *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++) * (dest + i) = * (source + i);
}

void set_plane(unsigned p) {
    unsigned char pmask;

    p &= 3;
    pmask = 1 << p;
    // set read plane
    port_byte_out(VGA_GC_INDEX, 4);
    port_byte_out(VGA_GC_DATA, p);
    // set write plane
    port_byte_out(VGA_SEQ_INDEX, 2);
    port_byte_out(VGA_SEQ_DATA, pmask);
}

/*************************
 * VGA public functions *
*************************/

void vga_put_pixel(unsigned x, unsigned y, unsigned c) {
    unsigned off, mask, pmask;
    if (x >= vga_width || y >= vga_height)
        return;
    if (vga_note == 1) {
        vpokeb(vga_width * y + x, c);
    }
    else if (vga_note == 2) {
        // need to be rewritten faster
        off = vga_width / 8 * y + x / 8;
        mask = 0x80 >> (x & 7) * 1;
        pmask = 1;
        for (unsigned p = 0; p < 4; p++) {
            set_plane(p);
            if (pmask & c) vpokeb(off, vpeekb(off) | mask);
            else vpokeb(off, vpeekb(off) & ~mask);
            pmask <<= 1;
        }
    }
}

void vga_pixel_clear() {
    for (unsigned int xy = 0; xy < vga_height * vga_width; xy++) {
        vga_put_pixel(xy % vga_width, xy / vga_width, 0x0F);
    }
}

void vga_320_mode() {
    if (vga_note == 1) return;

    int need_save = !vga_note;
    // setup the vga struct
    vga_width  = 320;
    vga_height = 200;
    vga_bpp    = 256;
    vga_note   = 1;

    // setup the vga registers
    write_registers(g_320x200x256);

    // make copy of framebuffer_segment (for font)
    if (need_save) {
        framebuffer_segment_copy = calloc(MEM_CPY_SIZE);
        mcp((void *) MEM_CPY_BASE, framebuffer_segment_copy, MEM_CPY_SIZE);
    }

    // clears the screen
    vga_pixel_clear();
}

void vga_640_mode() {
    if (vga_note == 2) return;

    int need_save = !vga_note;
    // setup the vga struct
    vga_width  = 640;
    vga_height = 480;
    vga_bpp    = 16;
    vga_note   = 2;

    // setup the vga registers
    write_registers(g_640x480x16);

    // make copy of framebuffer_segment (for font)
    if (need_save) {
        framebuffer_segment_copy = calloc(MEM_CPY_SIZE);
        mcp((void *) MEM_CPY_BASE, framebuffer_segment_copy, MEM_CPY_SIZE);
    }

    // clears the screen
    vga_pixel_clear();
}

void vga_text_mode() {
    if (vga_note == 0) return;
    vga_320_mode();

    // restore the framebuffer_segment
    mcp(framebuffer_segment_copy, (void *) MEM_CPY_BASE, MEM_CPY_SIZE);
    free(framebuffer_segment_copy);

    vga_width  = 80;
    vga_height = 25;
    vga_bpp    = 16;
    vga_note   = 0;

    // setup the vga registers
    write_registers(g_80x25_text);

    clear_screen();
}

/**********************
 * VGA api functions *
**********************/

int vga_get_width() {
    return vga_width;
}

int vga_get_height() {
    return vga_height;
}

void vga_print(int x, int y, char msg[], int big, unsigned color) {
    unsigned char *glyph, *font;
    font = big ? g_8x16_font : g_8x8_font;
    for (int i = 0; msg[i] != '\0'; i++) {
        glyph = font + (msg[i] * (big ? 16 : 8));
        for (int j = 0; j < (big ? 16 : 8); j++) {
            for (int k = 0; k < 8; k++) {
                if (!(glyph[j] & (1 << k))) continue;
                vga_put_pixel(i * 8 + x + 8 - k , y + j, color);
            }
        }
    }
}

void vga_draw_line(int x1, int y1, int x2, int y2, unsigned color) {
    int dx = x2 - x1;
    int dy = y2 - y1;
    int steps = abs(dx) > abs(dy) ? abs(dx) : abs(dy);
    float xinc = dx / (float) steps;
    float yinc = dy / (float) steps;
    float x = x1;
    float y = y1;
    for (int i = 0; i <= steps; i++) {
        vga_put_pixel(x, y, color);
        x += xinc;
        y += yinc;
    }
}

void vga_draw_rect(int x, int y, int w, int h, unsigned color) {
    for (int i = 0; i < w; i++) {
        for (int j = 0; j < h; j++) {
            vga_put_pixel(x + i, y + j, color);
        }
    }
}
