#include <i_ocmlib.h>
#include <i_time.h>

#include <syscall.h>
#include <string.h>
#include <stdio.h>


// we need the stdarg of the stdlib
#include <stdarg.h>

// input() setings
#define SLEEP_T 15
#define FIRST_L 12

// keyboard scancodes
#define SC_MAX 57

#define LSHIFT 42
#define RSHIFT 54
#define LEFT 75
#define RIGHT 77
#define OLDER 72
#define NEWER 80
#define BACKSPACE 14
#define DEL 83
#define ENTER 28
#define RESEND 224

// private functions

int main() {
    return 0;
}

int clean_buffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) buffer[i] = '\0';
    return 0;
}

void rainbow_print(char message[]) {
    char rainbow_colors[] = {c_green, c_cyan, c_blue, c_magenta, c_red, c_yellow};
    int i = 0;
    
    char buffer[2];
    buffer[1] = '\0';

    while (message[i] != 0) {
        buffer[0] = message[i];
        c_ckprint(buffer, rainbow_colors[i % 6]);
        i++;
    }
}

/***********************
 * INPUT PUBLIC FUNCS *
***********************/

void input_wh(char out_buffer[], int size, char color, char ** history, int history_size) {
    (void) color;
    (void) history;
    int sc;

    clean_buffer(out_buffer, size);

    do {
        sc = c_kb_get_scfh();
    } while (sc == ENTER);

    c_kb_reset_history();

    int buffer_index = 0;
    int buffer_actual_size = 0;
    int shift_pressed = 0;

    puts("$0");

    while (sc != ENTER && size > buffer_actual_size + 1) {
        ms_sleep(SLEEP_T);

        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            continue;
        }

        if (sc == BACKSPACE) {
            if (buffer_index > 0) {
                buffer_index--;
                buffer_actual_size--;
                out_buffer[buffer_index] = '\0';
                puts("\b \b");
            }
            continue;
        }

        // check if shift is pressed
        if (sc == LSHIFT || sc == RSHIFT) {
            shift_pressed = 1;
            continue;
        }

        // check if shift is released
        if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift_pressed = 0;
            continue;
        }

        // check if the scancode is a valid one
        if (sc > SC_MAX) {
            continue;
        }

        // convert the scancode to a char
        char c = c_kb_scancode_to_char(sc, shift_pressed);

        // check if the char is printable
        if (c < 32 || c > 126 || c == '?') {
            continue;
        }

        // print the char
        putchar(c);
        out_buffer[buffer_index] = c;

        buffer_actual_size++;
        buffer_index++;
    }

    out_buffer[buffer_index] = '\0';   
}

void input(char out_buffer[], int size, char color) {
    input_wh(out_buffer, size, color, NULL, 0);
}
