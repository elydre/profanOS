#include <syscall.h>
#include <stdio.h>

#include <i_iolib.h>
#include <i_time.h>

#define SC_MAX 57

int main(int argc, char **argv) {
    rainbow_print("enter scancode, press ESC to exit\n");
    int last_sc = 0;
    while (last_sc != 1) {
        while (last_sc == c_kb_get_scancode()) ms_sleep(10);
        last_sc = c_kb_get_scancode();
        printf("\r$4sc: $1%d $4($10x%x$4) letter: $1%c$4, $1%c    ",
                last_sc, last_sc,
                c_kb_scancode_to_char(last_sc, 0),
                c_kb_scancode_to_char(last_sc, 1)
        );
    }
    printf("\n");
    return 0;
}
