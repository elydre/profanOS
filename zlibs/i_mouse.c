#include <syscall.h>

int main() {
    return 0;
}

int mouse_get_x() {
    return c_mouse_call(0, 0);
}

int mouse_get_y() {
    return c_mouse_call(1, 0);
}

int mouse_get_button(int button) {
    return c_mouse_call(2, button);
}

void mouse_set_x(int x) {
    c_mouse_call(3, x);
}

void mouse_set_y(int y) {
    c_mouse_call(4, y);
}

void mouse_reset() {
    c_mouse_call(5, 0);
}
