#include <kernel/snowflake.h>
#include <gui/gnrtx.h>

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

void int2str(int n, char s[]) {
    int i = 0;
    do {
        s[i++] = n % 10 + '0';
    } while ((n /= 10) > 0);
    s[i] = '\0';
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
    // TODO: recode this
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

// formated print

void kprintf(char *fmt, ...) {
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
            }
        } else {
            char_buffer[buffer_i] = fmt[i];
            buffer_i++;
        }
        i++;
    }
    char_buffer[buffer_i] = '\0';
    kprint(char_buffer);
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
