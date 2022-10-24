#include "syscall.h"

#define MAX_ITER 64
#define LARGEUR c_vga_get_width()
#define HAUTEUR c_vga_get_height()

#define XMIN -2.4
#define XMAX 1
#define YMIN -1.25
#define YMAX 1.25

int main(int argc, char **argv) {
    c_vga_320_mode();
    c_vgui_setup(0);

    while (1) {
        long double cx, cy, xn, yn, tmp_x, tmp_y;
        int n;

        for (int y = 0; y < HAUTEUR; y++) {
            for (int x = 0; x < LARGEUR; x++) {
                cx = (x * (XMAX - XMIN) / LARGEUR + XMIN);
                cy = (y * (YMIN - YMAX) / HAUTEUR + YMAX);
                xn = 0;
                yn = 0;
                n = 0;
                while ((xn * xn + yn * yn) < 4 && n < MAX_ITER) {
                    if (c_kb_get_scancode() == 1) {
                        c_vgui_exit();
                        c_vga_text_mode();
                        return 0;
                    }
                    tmp_x = xn;
                    tmp_y = yn;
                    xn = tmp_x * tmp_x - tmp_y * tmp_y + cx;
                    yn = 2 * tmp_x * tmp_y + cy;
                    n++;
                }
                c_vgui_set_pixel(x, y, n);
            }
        }
        c_vgui_print(5, 5, "press escape to exit", 0, 63);
        c_vgui_render();
    }
}
