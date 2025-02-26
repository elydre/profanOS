/*****************************************************************************\
|   === vesa.c : 2024 ===                                                     |
|                                                                             |
|    Kernel VESA framebuffer driver                                .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/multiboot.h>
#include <gui/vesa.h>

uint32_t *framebuffer;
uint32_t fb_w, fb_h, fb_p;
int is_vesa;

void init_vesa(void) {
    framebuffer = (uint32_t *) g_mboot->framebuffer_addr_low;
    fb_p = g_mboot->framebuffer_pitch / sizeof(uint32_t);
    fb_w = g_mboot->framebuffer_width;
    fb_h = g_mboot->framebuffer_height;

    is_vesa = ((uint32_t) framebuffer > 0xb8000 && fb_w > 160 && fb_h > 100);
}

void vesa_set_pixel(int x, int y, uint32_t c) {
    framebuffer[y * fb_p + x] = c;
}

uint32_t vesa_get_info(int i) {
    switch (i) {
        case 0: return fb_w;
        case 1: return fb_h;
        case 2: return fb_p;
        case 3: return (uint32_t) framebuffer;
        case 4: return is_vesa;
        default: return 0;
    }
}
