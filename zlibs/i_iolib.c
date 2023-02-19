#include <syscall.h>
#include <i_string.h>
#include <i_time.h>
#include <i_mem.h>
#include <string.h>


// we need the stdarg of the stdlib
#include <stdarg.h>

// input() setings
#define SLEEP_T 30
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

char sprint_function(char message[], char default_color) {
    unsigned int msg_len = strlen(message);
    char nm[msg_len + 1];
    for (unsigned int i = 0; i < msg_len; i++) nm[i] = message[i];
    nm[msg_len] = '$'; nm[msg_len + 1] = '\0';
    msg_len++;
    char color = default_color;
    char buffer[msg_len];
    int buffer_index = 0;

    char delimiter[] = "0123456789ABCDE";
    char colors[] = {
        c_blue, c_green, c_cyan, c_red, c_magenta, c_yellow, c_grey, c_white, 
        c_dblue, c_dgreen, c_dcyan, c_dred, c_dmagenta, c_dyellow, c_dgrey
    };

    clean_buffer(buffer, msg_len);
    for (unsigned int i = 0; i < msg_len; i++) {
        if (nm[i] != '$') {
            buffer[buffer_index] = nm[i];
            buffer_index++;
            continue;
        }
        if (strlen(buffer) > 0) {
            c_ckprint(buffer, color);
            buffer_index = clean_buffer(buffer, msg_len);
        }
        if (i == msg_len - 1) break;
        for (int j = 0; j < ARYLEN(delimiter); j++) {
            if (nm[i + 1] == delimiter[j]) {
                color = colors[j];
                i++;
                break;
            }
        }
    }
    return color;
}

/***************************
 * PRINT PUBLIC FUNCTIONS  *
***************************/

void msprint(int nb_args, ...) {
    va_list args;
    va_start(args, nb_args);
    char last_color = c_white;
    for (int i = 0; i < nb_args; i++) {
        char *arg = va_arg(args, char*);
        last_color = sprint_function(arg, last_color);
    }
    va_end(args);
}

void vfsprint(char format[], va_list args) {
    char *buffer = malloc(0x1000);
    clean_buffer(buffer, 0x1000);
    char color = c_white;

    for (unsigned int i = 0; i <= strlen(format); i++) {
        if (i == strlen(format)) {
            sprint_function(buffer, color);
            continue;
        }
        if (format[i] != '%') {
            str_append(buffer, format[i]);
            continue;
        }
        color = sprint_function(buffer, color);
        clean_buffer(buffer, 0x1000);
        i++;
        if (format[i] == 's') {
            char *arg = va_arg(args, char*);
            for (unsigned int j = 0; j < strlen(arg); j++) buffer[j] = arg[j];
            buffer[strlen(arg)] = '\0';
            color = sprint_function(buffer, color);
        }
        else if (format[i] == 'd') {
            int arg = va_arg(args, int);
            int_to_ascii(arg, buffer);
            color = sprint_function(buffer, color);
        }
        else if (format[i] == 'x') {
            int arg = va_arg(args, int);
            hex_to_ascii(arg, buffer);
            color = sprint_function(buffer, color);
        }
        else if (format[i] == 'c') {
            char arg = va_arg(args, int);
            buffer[0] = arg;
            buffer[1] = '\0';
            color = sprint_function(buffer, color);
        }
        else if (format[i] == 'f') {
            double arg = va_arg(args, double);
            double_to_ascii(arg, buffer);
            color = sprint_function(buffer, color);
        }
        else i--;
        clean_buffer(buffer, 0x1000);
        continue;
    }
    free(buffer);
}

void fsprint(char format[], ...) {
    // how many % is there
    int nb_args = 0;
    for (unsigned int i = 0; i < strlen(format); i++) {
        if (format[i] == '%') nb_args++;
    }
    if (nb_args == 0) {
        sprint_function(format, c_white);
        return;
    }
    va_list args;
    va_start(args, format);
    vfsprint(format, args);
    va_end(args);
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

        if (key_ticks < FIRST_L && key_ticks) continue;

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

void input(char out_buffer[], int size, char color) {
    input_wh(out_buffer, size, color, NULL, 0);
}
