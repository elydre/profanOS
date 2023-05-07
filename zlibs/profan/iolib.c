#include <i_ocmlib.h>
#include <i_time.h>

#include <syscall.h>
#include <string.h>
#include <stdio.h>

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

/***************************
 * PRINT PUBLIC FUNCTIONS  *
***************************/

uint32_t color_print(char *s) {
    uint32_t msg_len = strlen(s);

    uint32_t from = 0;
    uint32_t i = 0;
    char c = c_white;

    while (s[i] != '\0') {
        for (i = from; i < msg_len - 1; i++) {
            if (s[i] != '$') continue;
            s[i] = '\0';
            c_ckprint(s + from, c);
            s[i] = '$';
            switch (s[i + 1]) {
                case '0':
                    c = c_blue;
                    break;
                case '1':
                    c = c_green;
                    break;
                case '2':
                    c = c_cyan;
                    break;
                case '3':
                    c = c_red;
                    break;
                case '4':
                    c = c_magenta;
                    break;
                case '5':
                    c = c_yellow;
                    break;
                case '6':
                    c = c_grey;
                    break;
                case '7':
                    c = c_white;
                    break;
                case '8':
                    c = c_dblue;
                    break;
                case '9':
                    c = c_dgreen;
                    break;
                case 'A':
                    c = c_dcyan;
                    break;
                case 'B':
                    c = c_dred;
                    break;
                case 'C':
                    c = c_dmagenta;
                    break;
                case 'D':
                    c = c_dyellow;
                    break;
                case 'E':
                    c = c_dgrey;
                    break;
                default:
                    c = c_white;
                    break;
            }
            i += 2;
            from = i;
            break;
        }
        i++;
    }
    c_ckprint(s + from, c);
    return msg_len;
}


void rainbow_print(char message[]) {
    char rainbow_colors[] = "120435";

    for (int i = 0; message[i] != 0; i++) {
        printf("$%c%c", rainbow_colors[i % 6], message[i]);
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
