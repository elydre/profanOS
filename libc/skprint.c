#include <skprint.h>
#include <string.h>
#include <screen.h>
#include <mem.h>

#include <stdarg.h>

int clean_buffer(char *buffer, int size) {
    for (int i = 0; i < size; i++) {
        buffer[i] = '\0';
    }
    return 0;
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
        if (nm[i] == '$') {
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
        else {
            buffer[buffer_index] = nm[i];
            buffer_index++;
        }
    }
    return color;
}

void skprint(char message[]) {
    skprint_function(message, c_white);
}

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
        if (format[i] == '%') {
            color = skprint_function(buffer, color);
            clean_buffer(buffer, 0x1000);
            i++;
            if (format[i] == 's') {
                char *arg = va_arg(args, char*);
                for (int j = 0; j < strlen(arg); j++) {
                    buffer[j] = arg[j];
                }
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
        append(buffer, format[i]);
    }
    free((int) buffer);
    va_end(args);
}
