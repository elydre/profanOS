#include <kernel/snowflake.h>
#include <driver/serial.h>
#include <driver/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <minilib.h>

// string functions

void str_cat(char s1[], char s2[]) {
    char *start = s1;
    while(*start != '\0') start++;
    while(*s2 != '\0') *start++ = *s2++;
    *start = '\0';
}

int str_len(char s[]) {
    int i = 0;
    while (s[i] != '\0') i++;
    return i;
}

void str_cpy(char s1[], char s2[]) {
    int i;
    for (i = 0; s2[i] != '\0'; i++) {
        s1[i] = s2[i];
    }
    s1[i] = '\0';
}

void str_reverse(char s[]) {
    int i = 0;
    int j = str_len(s) - 1;
    char tmp;
    while (i < j) {
        tmp = s[i];
        s[i] = s[j];
        s[j] = tmp;
        i++;
        j--;
    }
}

void int2str(int n, char s[]) {
    int i, sign;
    if ((sign = n) < 0) n = -n;
    i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);

    if (sign < 0) s[i++] = '-';
    s[i] = '\0';

    str_reverse(s);
}

void hex2str(uint32_t n, char s[]) {
    int i = 0;
    int tmp;
    char hex[] = "0123456789abcdef";
    do {
        tmp = n % 16;
        s[i++] = hex[tmp];
    } while ((n /= 16) > 0);
    s[i] = '\0';
    str_reverse(s);
}

int str2int(char s[]) {
    int i = 0;
    int n = 0;
    while (s[i] >= '0' && s[i] <= '9') {
        n = 10 * n + (s[i++] - '0');
    }
    return n;
}

int str_cmp(char s1[], char s2[]) {
    int i = 0;
    while (s1[i] == s2[i]) {
        if (s1[i] == '\0') return 0;
        i++;
    }
    return s1[i] - s2[i];
}

int str_ncmp(char s1[], char s2[], int n) {
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (i == n || s1[i] == '\0') return 0;
    }
    if (i == n) return 0;
    return s1[i] - s2[i];
}

int str_count(char s[], char c) {
    int i = 0;
    int count = 0;
    while (s[i] != '\0') {
        if (s[i] == c) count++;
        i++;
    }
    return count;
}

void str_append(char s[], char c) {
    int i = 0;
    while (s[i] != '\0') i++;
    s[i] = c;
    s[i+1] = '\0';
}

// formated print (multiple output)

void func_printf(int output, char *fmt, ...) {
    // printf kernel level
    // don't use va
    char *args = (char *) &fmt;
    args += 4;
    int i = 0;
    char char_buffer[256];
    int buffer_i = 0;
    while (fmt[i] != '\0') {
        if (fmt[i] == '%') {
            i++;
            if (fmt[i] == 's') {
                char *s = *((char **) args);
                args += 4;
                for (int j = 0; s[j] != '\0'; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }                
            } else if (fmt[i] == 'c') {
                char c = *((char *) args);
                args += 4;
                char_buffer[buffer_i] = c;
                buffer_i++;
            } else if (fmt[i] == 'd') {
                int n = *((int *) args);
                args += 4;
                char s[20];
                int2str(n, s);
                for (int j = 0; s[j] != '\0'; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == 'x') {
                uint32_t n = *((int *) args);
                args += 4;
                char s[20];
                hex2str(n, s);
                for (int j = 0; s[j] != '\0'; j++) {
                    char_buffer[buffer_i] = s[j];
                    buffer_i++;
                }
            } else if (fmt[i] == '%') {
                char_buffer[buffer_i] = '%';
                buffer_i++;
            }
        } else {
            char_buffer[buffer_i] = fmt[i];
            buffer_i++;
        }
        i++;
    }
    char_buffer[buffer_i] = '\0';
    if (output == 0) {
        kprint(char_buffer);
    } else if (output == 1) {
        serial_print(SERIAL_PORT_A, char_buffer);
    }
}

// memory management

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes) {
    for (int i = 0; i < nbytes; i++) *(dest + i) = *(source + i);
}

void mem_set(uint8_t *dest, uint8_t val, uint32_t len) {
    uint8_t *temp = (uint8_t *)dest;
    for ( ; len != 0; len--) *temp++ = val;
}

void mem_move(uint8_t *source, uint8_t *dest, int nbytes) {
    if (source < dest) {
        for (int i = nbytes - 1; i >= 0; i--) {
            *(dest + i) = *(source + i);
        }
    } else {
        for (int i = 0; i < nbytes; i++) {
            *(dest + i) = *(source + i);
        }
    }
}

void free(void *addr) {
    int size = mem_get_alloc_size((uint32_t) addr);
    if (size == 0) return;
    mem_set((uint8_t *) addr, 0, size);
    mem_free_addr((int) addr);
}

void *malloc(uint32_t size) {
    uint32_t addr = mem_alloc(size, 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc(void *ptr, uint32_t size) {
    uint32_t addr = (uint32_t) ptr;
    uint32_t new_addr = mem_alloc(size, 1);
    if (new_addr == 0) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, size);
    mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(uint32_t size) {
    int addr = mem_alloc(size, 1);
    if (addr == 0) return NULL;
    mem_set((uint8_t *) addr, 0, size);
    return (void *) addr;
}

// status print

void status_print(int (*func)(), char *verb, char *noun) {
    int old_cursor, new_cursor, status;

    ckprint("[", 0x0F);
    old_cursor = get_cursor_offset();
    ckprint("WORK", 0x0E);
    ckprint("]  ", 0x0F);
    ckprint(verb, 0x07);
    ckprint(" ", 0x0F);
    ckprint(noun, 0x0F);
    ckprint("\n", 0x0F);

    status = func();
    new_cursor = get_cursor_offset();
    set_cursor_offset(old_cursor);

    if (status == 0) {
        ckprint(" OK ", 0x0A);
    } else if (status == 2) {
        ckprint("ENBL", 0x0B);
    } else {
        ckprint("FAIL", 0x0C);
    }

    set_cursor_offset(new_cursor);
}

// random

uint32_t rand_val = 0;

int init_rand() {
    i_time_t time;
    time_get(&time);
    rand_val = time.seconds + time.minutes * 60 + time.hours * 60 * 60;
    return 0;
}

uint32_t rand() {
    rand_val = rand_val * 1103515245 + 12345;
    return (uint32_t) rand_val;
}

