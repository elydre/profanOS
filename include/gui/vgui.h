#ifndef VGUI_H
#define VGUI_H

/*********************************
 * VGUI - virtual graphical lib *
 * vgui can be  used to draw on *
 * the  screen without  lagging *
*********************************/

void vgui_setup(int refresh_all);
void vgui_exit();
void vgui_render();

int vgui_get_refresh_mode();

void vgui_set_pixel(int x, int y, int color);
int  vgui_get_pixel(int x, int y);

void vgui_draw_rect(int x, int y, int width, int height, int color);
void vgui_draw_line(int x1, int y1, int x2, int y2, int color);
void vgui_print(int x, int y, char msg[], int big, unsigned color);
void vgui_clear(int color);

#endif

