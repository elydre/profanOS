#include <syscall.h>
#include <string.h>

#include <i_time.h>


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
    int old_cursor = c_get_cursor_offset();
    int sc, last_sc, last_sc_sgt = 0;
    int buffer_actual_size = 0;
    int history_index = 0;
    int buffer_index = 0;
    int key_ticks = 0;
    int shift = 0;
    int new_pos;

    int row = c_gt_get_max_rows();
    int col = c_gt_get_max_cols();

    clean_buffer(out_buffer, size);

    do {
        sc = c_kb_get_scfh();
    } while (sc == ENTER);

    c_kb_reset_history();

    c_cursor_blink(1);

    while (sc != ENTER) {
        ms_sleep(SLEEP_T);

        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        if (!sc) continue;
        if (sc != last_sc) key_ticks = 0;
        else key_ticks++;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) continue;

        if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
            continue;
        }

        else if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
            continue;
        }
        
        else if (sc == LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == OLDER) {
            if (history_index == history_size) continue;
            c_set_cursor_offset(old_cursor);
            for (int i = 0; i < buffer_actual_size; i++) c_kprint(" ");
            clean_buffer(out_buffer, size);
            buffer_actual_size = ((int) strlen(history[history_index]) > size) ? size : (int) strlen(history[history_index]);
            for (int i = 0; i < buffer_actual_size; i++) out_buffer[i] = history[history_index][i];
            buffer_index = buffer_actual_size;
            history_index++;
        }

        else if (sc == NEWER) {
            clean_buffer(out_buffer, size);
            c_set_cursor_offset(old_cursor);
            for (int i = 0; i < buffer_actual_size; i++) c_kprint(" ");
            if (history_index < 2) {
                buffer_actual_size = 0;
                buffer_index = 0;
                continue;
            }
            history_index--;
            buffer_actual_size = ((int) strlen(history[history_index - 1]) > size) ? size : (int) strlen(history[history_index - 1]);
            for (int i = 0; i < buffer_actual_size; i++) out_buffer[i] = history[history_index - 1][i];
            buffer_index = buffer_actual_size;
        }

        else if (sc == BACKSPACE) {
            if (!buffer_index) continue;
            buffer_index--;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                out_buffer[i] = out_buffer[i + 1];
            }
            out_buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == DEL) {
            if (!buffer_index || buffer_index == buffer_actual_size) continue;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                out_buffer[i] = out_buffer[i + 1];
            }
            out_buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == KB_TAB) {
            if (size < buffer_actual_size + 5) continue;
            for (int i = 0; i < 4; i++) {
                out_buffer[buffer_index] = ' ';
                buffer_actual_size++;
                buffer_index++;
            }
        }

        else if (sc <= SC_MAX) {
            if (size < buffer_actual_size + 2) continue;
            if (c_kb_scancode_to_char(sc, shift) == '?') continue;
            for (int i = buffer_actual_size; i > buffer_index; i--) {
                out_buffer[i] = out_buffer[i - 1];
            }
            out_buffer[buffer_index] = c_kb_scancode_to_char(sc, shift);
            buffer_actual_size++;
            buffer_index++;
        }

        c_set_cursor_offset(old_cursor);
        c_ckprint(out_buffer, color);
        c_kprint(" ");
        new_pos = old_cursor + buffer_index * 2;
        c_set_cursor_offset(new_pos);
        if (new_pos >= (row * col - 1) * 2) {
            old_cursor -= row * 2;
        }
    }
    c_cursor_blink(0);
}
