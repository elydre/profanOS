#include <driver/keyboard.h>
#include <string.h>
#include <iolib.h>
#include <time.h>
#include <mem.h>

// we need the stdarg of the stdlib
#include <stdarg.h>

// input() setings
#define FIRST_L 40
#define BONDE_L 4

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

// private functions

int clean_buffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) buffer[i] = '\0';
    return 0;
}

char skprint_function(char message[], char default_color) {
    int msg_len = str_len(message);
    char nm[msg_len + 1];
    for (int i = 0; i < msg_len; i++) nm[i] = message[i];
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
    for (int i = 0; i < msg_len; i++) {
        if (nm[i] != '$') {
            buffer[buffer_index] = nm[i];
            buffer_index++;
            continue;
        }
        if (str_len(buffer) > 0) {
            ckprint(buffer, color);
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
 * PRINT PUBLIC FUNCTIONS *
***************************/

void mskprint(int nb_args, ...) {
    va_list args;
    va_start(args, nb_args);
    char last_color = c_white;
    for (int i = 0; i < nb_args; i++) {
        char *arg = va_arg(args, char*);
        last_color = skprint_function(arg, last_color);
    }
    va_end(args);
}

void fskprint(char format[], ...) {
    va_list args;
    va_start(args, format);
    char *buffer =0x300000; // TODO: use malloc
    clean_buffer(buffer, 0x1000);
    char color = c_white;

    for (int i = 0; i <= str_len(format); i++) {
        if (i == str_len(format)) {
            skprint_function(buffer, color);
            continue;
        }
        if (format[i] != '%') {
            str_append(buffer, format[i]);
            continue;
        }
        color = skprint_function(buffer, color);
        clean_buffer(buffer, 0x1000);
        i++;
        if (format[i] == 's') {
            char *arg = va_arg(args, char*);
            for (int j = 0; j < str_len(arg); j++) buffer[j] = arg[j];
            buffer[str_len(arg)] = '\0';
            color = skprint_function(buffer, color);
        }
        else if (format[i] == 'd') {
            int arg = va_arg(args, int);
            int_to_ascii(arg, buffer);
            color = skprint_function(buffer, color);
        }
        else if (format[i] == 'x') {
            int arg = va_arg(args, int);
            hex_to_ascii(arg, buffer);
            color = skprint_function(buffer, color);
        }
        else if (format[i] == 'c') {
            char arg = va_arg(args, int);
            buffer[0] = arg;
            buffer[1] = '\0';
            color = skprint_function(buffer, color);
        }
        else if (format[i] == 'f') {
            double arg = va_arg(args, double);
            double_to_ascii(arg, buffer);
            color = skprint_function(buffer, color);
        }
        else i--;
        clean_buffer(buffer, 0x1000);
        continue;
    }
    // free(buffer);
    va_end(args);
}

void rainbow_print(char message[]) {
    char rainbow_colors[] = {c_green, c_cyan, c_blue, c_magenta, c_red, c_yellow};
    int i = 0;
    
    char buffer[2];
    buffer[1] = '\0';

    while (message[i] != 0) {
        buffer[0] = message[i];
        ckprint(buffer, rainbow_colors[i % 6]);
        i++;
    }
}

/***********************
 * INPUT PUBLIC FUNCS *
***********************/

void input_wh(char out_buffer[], int size, char color, char ** history, int history_size) {
    int old_cursor = get_cursor_offset();
    int buffer_actual_size = 0;
    int history_index = 0;
    int buffer_index = 0;
    int key_ticks = 0;
    int sc, last_sc;
    int shift = 0;
    int new_pos;

    int row = gt_get_max_rows();
    int col = gt_get_max_cols();

    clean_buffer(out_buffer, size);

    do {
        sc = kb_get_scfh();
    } while (sc == ENTER);

    kb_reset_history();

    while (sc != ENTER) {
        ms_sleep(2);

        last_sc = sc;
        sc = kb_get_scfh();
        
        if (sc != last_sc) key_ticks = 0;
        else key_ticks++;

        if ((key_ticks > 2 && key_ticks < FIRST_L) || key_ticks % BONDE_L) continue;

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
            set_cursor_offset(old_cursor);
            for (int i = 0; i < buffer_actual_size; i++) kprint(" ");
            clean_buffer(out_buffer, size);
            buffer_actual_size = (str_len(history[history_index]) > size) ? size : str_len(history[history_index]);
            for (int i = 0; i < buffer_actual_size; i++) out_buffer[i] = history[history_index][i];
            buffer_index = buffer_actual_size;
            history_index++;
        }

        else if (sc == NEWER) {
            clean_buffer(out_buffer, size);
            set_cursor_offset(old_cursor);
            for (int i = 0; i < buffer_actual_size; i++) kprint(" ");
            if (history_index < 2) {
                buffer_actual_size = 0;
                buffer_index = 0;
                continue;
            }
            history_index--;
            buffer_actual_size = (str_len(history[history_index - 1]) > size) ? size : str_len(history[history_index - 1]);
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
            if (kb_scancode_to_char(sc, shift) == '?') continue;
            for (int i = buffer_actual_size; i > buffer_index; i--) {
                out_buffer[i] = out_buffer[i - 1];
            }
            out_buffer[buffer_index] = kb_scancode_to_char(sc, shift);
            buffer_actual_size++;
            buffer_index++;
        }


        set_cursor_offset(old_cursor);
        ckprint(out_buffer, color);
        kprint(" ");
        new_pos = old_cursor + buffer_index * 2;
        set_cursor_offset(new_pos);
        if (new_pos >= (row * col - 1) * 2) {
            old_cursor -= row * 2;
        }
    }
}

void input(char out_buffer[], int size, char color) {
    input_wh(out_buffer, size, color, NULL, 0);
}
