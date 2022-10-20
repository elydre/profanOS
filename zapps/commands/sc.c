#include <syscall.h>
#define SC_MAX 57

int main(int argc, char **argv) {
    c_clear_screen();
    c_rainbow_print("enter scancode, press ESC to exit\n");
    int last_sc = 0;
    while (last_sc != 1) {
        while (last_sc == c_kb_get_scancode());
        last_sc = c_kb_get_scancode();
        c_fskprint("$4\nscancode:   $1%d", last_sc);

        if (last_sc > SC_MAX) {
            c_fskprint("\n$5unknown scancode\n");
            continue;
        }

        c_fskprint("$4\nletter min: $1%c", c_kb_scancode_to_char(last_sc, 0));
        c_fskprint("$4\nletter maj: $1%c\n", c_kb_scancode_to_char(last_sc, 1));
    }
    c_clear_screen();
    return 0;
}
