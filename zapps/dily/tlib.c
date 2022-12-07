#include "../syscall.h"

int main(void) {
    return 0;
}

int sum(int a, int b) {
    c_fskprint("sum called\n");
    return a + b;
}

void draw_rect(int x, int y, int w, int h, int color) {
    int i, j;
    for (i = 0; i < w; i++) {
        for (j = 0; j < h; j++) {
            c_vesa_set_pixel(x + i, y + j, color);
        }
    }
}

int mhh(int a, int b) {
    return a * b;
}
