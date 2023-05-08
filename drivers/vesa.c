#include <kernel/multiboot.h>
#include <gui/vesa.h>
#include <type.h>

uint32_t *framebuffer;
uint32_t fb_w, fb_h, fb_p;
int is_vesa;

void init_vesa() {
    framebuffer = (uint32_t *) mboot_get(22);
    fb_p = mboot_get(24) / sizeof(uint32_t);
    fb_w = mboot_get(25);
    fb_h = mboot_get(26);

    is_vesa = ((uint32_t) framebuffer > 0xb8000 && fb_w > 320 && fb_h > 200);
}

int vesa_does_enable() {
    return is_vesa;
}

void vesa_set_pixel(int x, int y, uint32_t c) {
    framebuffer[y * fb_p + x] = c;
}

// read from framebuffer can be slow
uint32_t vesa_get_pixel(int x, int y) {
    return framebuffer[y * fb_p + x];
}

void *vesa_get_framebuffer() {
    return framebuffer;
}

uint32_t vesa_get_pitch() {
    return fb_p;
}

uint32_t vesa_get_width() {
    return fb_w;
}

uint32_t vesa_get_height() {
    return fb_h;
}
