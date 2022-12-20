#ifndef VGUI_ID
#define VGUI_ID 1006

#define get_func_addr ((int (*)(int, int)) *(int *) 0x199994)


/*
int main();
void vgui_setup(int refresh_all);
void vgui_exit();
void vgui_render();
int vgui_get_refresh_mode();
void vgui_set_pixel(int x, int y, uint32_t color);
uint32_t vgui_get_pixel(int x, int y);
void vgui_draw_rect(int x, int y, int width, int height, uint32_t color);
void vgui_print(int x, int y, char msg[], uint32_t color);
int abs(int x);
void vgui_draw_line(int x1, int y1, int x2, int y2, uint32_t color);
void vgui_clear(uint32_t color);
*/


#define vgui_setup ((void (*)(int)) get_func_addr(VGUI_ID, 2))
#define vgui_exit ((void (*)()) get_func_addr(VGUI_ID, 3))
#define vgui_render ((void (*)()) get_func_addr(VGUI_ID, 4))
#define vgui_get_refresh_mode ((int (*)()) get_func_addr(VGUI_ID, 5))
#define vgui_set_pixel ((void (*)(int, int, uint32_t)) get_func_addr(VGUI_ID, 6))
#define vgui_get_pixel ((uint32_t (*)(int, int)) get_func_addr(VGUI_ID, 7))
#define vgui_draw_rect ((void (*)(int, int, int, int, uint32_t)) get_func_addr(VGUI_ID, 8))
#define vgui_print ((void (*)(int, int, char[], uint32_t)) get_func_addr(VGUI_ID, 9))
#define vgui_draw_line ((void (*)(int, int, int, int, uint32_t)) get_func_addr(VGUI_ID, 11))
#define vgui_clear ((void (*)(uint32_t)) get_func_addr(VGUI_ID, 12))

#endif
