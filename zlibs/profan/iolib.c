#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

#include <profan.h>
#include <panda.h>

// input() setings
#define SLEEP_T 15
#define FIRST_L 12

// keyboard scancodes
#define SC_MAX 57

#define LSHIFT 42
#define RSHIFT 54
#define LEFT 75
#define BACK   14
#define RIGHT 77
#define OLDER 72
#define NEWER 80
#define BACKSPACE 14
#define DEL 83
#define ESC 1
#define ENTER 28
#define RESEND 224

// private functions

int main(void) {
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

    while (i < msg_len) {
        for (i = from; i < msg_len - 1; i++) {
            if (s[i] != '$') continue;
            s[i] = '\0';
            c_kcprint(s + from, c);
            s[i] = '$';
            switch (s[i + 1]) {
                case '0': c = c_blue;     break;
                case '1': c = c_green;    break;
                case '2': c = c_cyan;     break;
                case '3': c = c_red;      break;
                case '4': c = c_magenta;  break;
                case '5': c = c_yellow;   break;
                case '6': c = c_grey;     break;
                case '7': c = c_white;    break;
                case '8': c = c_dblue;    break;
                case '9': c = c_dgreen;   break;
                case 'A': c = c_dcyan;    break;
                case 'B': c = c_dred;     break;
                case 'C': c = c_dmagenta; break;
                case 'D': c = c_dyellow;  break;
                case 'E': c = c_dgrey;    break;
                default:  c = c_white;    break;
            }
            i += 2;
            from = i;
            break;
        }
        i++;
    }
    if (from < msg_len)
        c_kcprint(s + from, c);
    return msg_len;
}

char panda_color_print(char *s, char c, uint32_t len) {
    uint32_t from = 0;
    uint32_t i = 0;

    while (i < len) {
        for (i = from; i < len - 1; i++) {
            if (s[i] != '$') continue;
            panda_print_string(s + from, i - from, c);
            switch (s[i + 1]) {
                case '0': c = c_blue;     break;
                case '1': c = c_green;    break;
                case '2': c = c_cyan;     break;
                case '3': c = c_red;      break;
                case '4': c = c_magenta;  break;
                case '5': c = c_yellow;   break;
                case '6': c = c_grey;     break;
                case '7': c = c_white;    break;
                case '8': c = c_dblue;    break;
                case '9': c = c_dgreen;   break;
                case 'A': c = c_dcyan;    break;
                case 'B': c = c_dred;     break;
                case 'C': c = c_dmagenta; break;
                case 'D': c = c_dyellow;  break;
                case 'E': c = c_dgrey;    break;
                default:  c = c_white;    break;
            }
            i += 2;
            from = i;
            break;
        }
        i++;
    }
    if (from < len)
        panda_print_string(s + from, i - from, c);
    return c;
}

void rainbow_print(char *message) {
    // char rainbow_colors[] = {c_green, c_cyan, c_blue, c_magenta, c_red, c_yellow};
    char rainbow_colors[] = {'1', '2', '0', '4', '3', '5'};
    int color_index = 0;

    char *new = malloc((strlen(message) + 1) * 3);
    // set color codes evry chars

    int i;
    for (i = 0; message[i]; i++) {
        new[i * 3] = '$';
        new[i * 3 + 1] = rainbow_colors[color_index];
        new[i * 3 + 2] = message[i];
        color_index = (color_index + 1) % 6;
    }

    new[i * 3] = '$';
    new[i * 3 + 1] = '7';
    new[i * 3 + 2] = '\0';

    fputs(new, stdout);
    free(new);
}

/***********************
 * INPUT PUBLIC FUNCS *
***********************/

uint32_t open_input(char *buffer, uint32_t size) {
    // save the current cursor position and show it
    printf("\033[s\033[?25l");
    fflush(stdout);

    int sc, last_sc, last_sc_sgt = 0;

    uint32_t buffer_actual_size = 0;
    uint32_t buffer_index = 0;

    for (uint32_t i = 0; i < size; i++)
        buffer[i] = '\0';

    int key_ticks = 0;
    int shift = 0;

    sc = 0;
    while (sc != ENTER) {
        usleep(SLEEP_T * 1000);
        sc = c_kb_get_scfh();

        if (sc == RESEND || sc == 0) {
            sc = last_sc_sgt;
        } else {
            last_sc_sgt = sc;
        }

        key_ticks = (sc != last_sc) ? 0 : key_ticks + 1;
        last_sc = sc;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            continue;
        }

        if (sc == LSHIFT || sc == RSHIFT) {
            shift = 1;
            continue;
        }

        if (sc == LSHIFT + 128 || sc == RSHIFT + 128) {
            shift = 0;
            continue;
        }

        if (sc == LEFT) {
            if (!buffer_index) continue;
            buffer_index--;
        }

        else if (sc == RIGHT) {
            if (buffer_index == buffer_actual_size) continue;
            buffer_index++;
        }

        else if (sc == BACK) {
            if (!buffer_index) continue;
            buffer_index--;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == DEL) {
            if (buffer_index == buffer_actual_size) continue;
            for (uint32_t i = buffer_index; i < buffer_actual_size; i++) {
                buffer[i] = buffer[i + 1];
            }
            buffer[buffer_actual_size] = '\0';
            buffer_actual_size--;
        }

        else if (sc == ESC) {
            return buffer_actual_size;
        }

        else if (sc <= SC_MAX) {
            if (size < buffer_actual_size + 2) continue;
            if (profan_kb_get_char(sc, shift) == '\0') continue;
            for (uint32_t i = buffer_actual_size; i > buffer_index; i--) {
                buffer[i] = buffer[i - 1];
            }
            buffer[buffer_index] = profan_kb_get_char(sc, shift);
            buffer_actual_size++;
            buffer_index++;
        }

        else continue;

        printf("\033[?25h\033[u$0%s $7\033[u\033[%dC\033[?25l", buffer, buffer_index);
        fflush(stdout);
    }

    buffer[buffer_actual_size++] = '\n';
    buffer[buffer_actual_size] = '\0';
    printf("\033[?25h\n");

    return buffer_actual_size;
}
