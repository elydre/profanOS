#include <libc/multiboot.h>
#include <gui/vesa.h>
#include <type.h>

uint32_t *framebuffer;
uint32_t fb_w, fb_h, fb_p, fb_bpp;

void init_vesa() {
	framebuffer = (uint32_t *) mboot_get(22);
	fb_p = mboot_get(24);
	fb_w = mboot_get(25);
	fb_h = mboot_get(26);
	fb_bpp = mboot_get(27) >> 24;
}

void vesa_set_pixel(int x, int y, uint32_t c) {
    framebuffer[y * fb_w + x] = c;
}

void vesa_clear_screen(uint32_t c) {
    for (uint32_t y = 0; y < fb_h; ++y) {
        for (uint32_t x = 0; x < fb_w; ++x) {
            vesa_set_pixel(x, y, c);
        }
    }
}
