#include <libc/multiboot.h>
#include <gui/vesa.h>
#include <type.h>

uint32_t *framebuffer;
uint32_t fb_w, fb_h, fb_p;
int is_vesa;

void init_vesa() {
	framebuffer = (uint32_t *) mboot_get(22);
	fb_p = mboot_get(24);
	fb_w = mboot_get(25);
	fb_h = mboot_get(26);

    is_vesa = ((uint32_t) framebuffer > 0xb8000 && fb_w > 320 && fb_h > 200);
}

int vesa_does_enable() {
    return is_vesa;
}

void vesa_set_pixel(int x, int y, uint32_t c) {
    framebuffer[y * fb_w + x] = c;
}
