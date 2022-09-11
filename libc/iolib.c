#include <driver/keyboard.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

#include <stdarg.h>

#define SC_MAX 57

#define LSHIFT 42
#define RSHIFT 54

#define LEFT 75
#define RIGHT 77

#define BACKSPACE 14
#define ENTER 28

// private functions

int clean_buffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) buffer[i] = '\0';
    return 0;
}

int in_table(int n, int list[], int list_len) {
    for (int i = 0; i < list_len; i++) {
        if (n == list[i]) return 1;
    }
    return 0;
}

void add_to_table(int n, int list[], int list_len) {
    for (int i = 0; i < list_len; i++) {
        if (list[i] != 0) continue;
        list[i] = n;
        return;
    }
}

ScreenColor skprint_function(char message[], ScreenColor default_color) {
    int msg_len = strlen(message);
    char nm[msg_len + 1];
    for (int i = 0; i < msg_len; i++) nm[i] = message[i];
    nm[msg_len] = '$'; nm[msg_len + 1] = '\0';
    msg_len++;
    ScreenColor color = default_color;
    char buffer[msg_len];
    int buffer_index = 0;

    char delimiter[] = "0123456789ABCDE";
    ScreenColor colors[] = {
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
        if (strlen(buffer) > 0) {
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

// PRINT public functions

void mskprint(int nb_args, ...) {
    va_list args;
    va_start(args, nb_args);
    ScreenColor last_color = c_white;
    for (int i = 0; i < nb_args; i++) {
        char *arg = va_arg(args, char*);
        last_color = skprint_function(arg, last_color);
    }
    va_end(args);
}

void fskprint(char format[], ...) {
    va_list args;
    va_start(args, format);
    char *buffer = malloc(0x1000);
    clean_buffer(buffer, 0x1000);
    ScreenColor color = c_white;

    for (int i = 0; i <= strlen(format); i++) {
        if (i == strlen(format)) {
            skprint_function(buffer, color);
            continue;
        }
        if (format[i] != '%') {
            append(buffer, format[i]);
            continue;
        }
        color = skprint_function(buffer, color);
        clean_buffer(buffer, 0x1000);
        i++;
        if (format[i] == 's') {
            char *arg = va_arg(args, char*);
            for (int j = 0; j < strlen(arg); j++) buffer[j] = arg[j];
            buffer[strlen(arg)] = '\0';
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
        else i--;
        clean_buffer(buffer, 0x1000);
        continue;
    }
    free((int) buffer);
    va_end(args);
}

void rainbow_print(char message[]) {
    ScreenColor rainbow_colors[] = {c_green, c_cyan, c_blue, c_magenta, c_red, c_yellow};
    int i = 0;

    while (message[i] != 0) {
        print_char(message[i], -1, -1, rainbow_colors[i % 6]);
        i++;
    }
}

void input_print_buffer(int old_cursor, char out_buffer[], ScreenColor color, int buffer_index) {
    set_cursor_offset(old_cursor);
    ckprint(out_buffer, color);
    ckprint(" ", color);
    set_cursor_offset(old_cursor + buffer_index * 2);
}

// INPUT public functions

void input(char out_buffer[], int size, ScreenColor color) {
    int old_cursor = get_cursor_offset();
    int buffer_actual_size = 0;
    int buffer_index = 0;
    int shift = 0;
    int sc = 0;
    char c;

    int not_exited_keys[10];
    for (int i = 0; i < 10; i++) not_exited_keys[i] = 0;
    clean_buffer(out_buffer, size);

    if (sc <= SC_MAX) add_to_table(sc, not_exited_keys, 10);
    while ((sc != ENTER || !buffer_index) && buffer_index < size) {
        sc = get_last_scancode();

        if (sc == LSHIFT || sc == RSHIFT) shift = 1;
        else if (sc == LSHIFT + 128 || sc == RSHIFT + 128) shift = 0;
        else if (in_table(sc, not_exited_keys, 10)) continue;

        else if (sc == LEFT) {
            add_to_table(sc, not_exited_keys, 10);
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == RIGHT) {
            add_to_table(sc, not_exited_keys, 10);
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == BACKSPACE) {
            add_to_table(sc, not_exited_keys, 10);
            if (!buffer_index) continue;
            buffer_index--;
            for (int i = buffer_index; i < buffer_actual_size; i++) {
                out_buffer[i] = out_buffer[i + 1];
            }
            out_buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc <= SC_MAX) {
            add_to_table(sc, not_exited_keys, 10);

            c = scancode_to_char(sc, shift);
            if (c == '?') continue;
            for (int i = buffer_actual_size; i > buffer_index; i--) {
                out_buffer[i] = out_buffer[i - 1];
            }
            out_buffer[buffer_index] = c;
            buffer_actual_size++;
            buffer_index++;
        }
        else {
            for (int i = 0; i < 10; i++) {
                if (not_exited_keys[i] != sc - 128) continue;
                not_exited_keys[i] = 0;
                break;
            }
            continue;
        }
        input_print_buffer(old_cursor, out_buffer, color, buffer_index);
    }
}
