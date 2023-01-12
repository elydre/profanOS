#ifndef MOUSE_ID
#define MOUSE_ID 1010

#define get_func_addr ((int (*)(int, int)) *(int *) 0x1ffffb)

/*
int main();
int mouse_get_x();
int mouse_get_y();
int mouse_get_button(int button);
void mouse_set_x(int x);
void mouse_set_y(int y);
void mouse_reset();
*/

#define mouse_get_x ((int (*)(void)) get_func_addr(MOUSE_ID, 2))
#define mouse_get_y ((int (*)(void)) get_func_addr(MOUSE_ID, 3))
#define mouse_get_button ((int (*)(int)) get_func_addr(MOUSE_ID, 4))
#define mouse_set_x ((void (*)(int)) get_func_addr(MOUSE_ID, 5))
#define mouse_set_y ((void (*)(int)) get_func_addr(MOUSE_ID, 6))
#define mouse_reset ((void (*)(void)) get_func_addr(MOUSE_ID, 7))

#endif
