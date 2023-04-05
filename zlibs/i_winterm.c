#include <syscall.h>
#include <stdlib.h>
#include <string.h>

#define BUFFER_SIZE 0x1000

char *char_buffer;
int curent_len;

int main() {
    char_buffer = calloc(BUFFER_SIZE, sizeof(char));
    curent_len = 0;

    return 0;
}

void wterm_append_string(char *str) {
    int len = strlen(str);
    if (curent_len + len > BUFFER_SIZE - 2) {
        for (int i = 0; i < BUFFER_SIZE - len; i++) {
            char_buffer[i] = char_buffer[i + len];
        }
        curent_len -= len;
    }

    for (int i = 0; i < len; i++) {
        char_buffer[curent_len + i] = str[i];
    }

    curent_len += len;
    char_buffer[curent_len] = 0;
}

void wterm_append_char(char c) {
    if (curent_len + 1 > BUFFER_SIZE - 2) {
        for (int i = 0; i < BUFFER_SIZE - 1; i++) {
            char_buffer[i] = char_buffer[i + 1];
        }
    }
    char_buffer[curent_len] = c;
    curent_len++;
    if (curent_len >= BUFFER_SIZE) {
        curent_len = BUFFER_SIZE - 1;
    }
    char_buffer[curent_len] = 0;
}

char *wterm_get_buffer() {
    return char_buffer;
}

int wterm_get_len() {
    return curent_len;
}
