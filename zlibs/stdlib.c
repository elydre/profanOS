#include <syscall.h>
#include <i_iolib.h>

void init_func();

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);
void mem_move(uint8_t *source, uint8_t *dest, int nbytes);

int main() {
    init_func();
    return 0;
}

void init_func() {
    c_kprint("Init of the stdlib !\n");
}

void *calloc(uint32_t nmemb, uint32_t lsize) {
    uint32_t size = lsize * nmemb;
    int addr = c_mem_alloc(size, 1);
    if (addr == 0) return NULL;
    mem_set((uint8_t *) addr, 0, size);
    return (void *) addr;
}

void free(void *mem) {
    int size = c_mem_get_alloc_size((uint32_t) mem);
    if (size == 0) return;
    mem_set((uint8_t *) mem, 0, size);
    c_mem_free_addr((int) mem);
}

void *malloc(uint32_t size) {
    uint32_t addr = c_mem_alloc(size, 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc(void *mem, uint32_t new_size) {
    uint32_t addr = (uint32_t) mem;
    uint32_t new_addr = c_mem_alloc(new_size, 1);
    if (new_addr == 0) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, new_size);
    c_mem_free_addr(addr);
    return (void *) new_addr;
}


#define TABLE_BASE 0x2e
#define TABLE_SIZE 0x4d
#define XX ((char)0x40)

static const char a64l_table[TABLE_SIZE] = {
  /* 0x2e */                                                           0,  1,
  /* 0x30 */   2,  3,  4,  5,  6,  7,  8,  9, 10, 11, XX, XX, XX, XX, XX, XX,
  /* 0x40 */  XX, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26,
  /* 0x50 */  27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, XX, XX, XX, XX, XX,
  /* 0x60 */  XX, 38, 39, 40, 41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, 52,
  /* 0x70 */  53, 54, 55, 56, 57, 58, 59, 60, 61, 62, 63
};


long int a64l(const char *string) {
    const char *ptr = string;
    unsigned long int result = 0ul;
    const char *end = ptr + 6;
    int shift = 0;

    do {
        unsigned index;
        unsigned value;

        index = *ptr - TABLE_BASE;
        if ((unsigned int) index >= TABLE_SIZE)
        break;
        value = (int) a64l_table[index];
        if (value == (int) XX)
            break;
        ++ptr;
        result |= value << shift;
        shift += 6;
    } while (ptr != end);

    return (long int) result;
}

void abort() {
    int pid = 0; // kernel pid for now
    c_mem_free_all(c_task_get_current_pid()); // free all memory
    c_task_kill_task_switch(pid); // kill the current task and go to kernel
}

int abs(int j) {
	return (j >= 0) ? j : -j;
}

void atexit(void (*func)()) {
    fsprint("atexit not implemented yet, WHY DO YOU USE IT ?\n");
    // TODO : create c_task_add_exit_func
    // c_task_add_exit_func(func); 
}

double atof(const char *s) {
    fsprint("atof not implemented yet, WHY DO YOU USE IT ?\n");
    return 0.0;
    // TODO : implement atof
    // This function stolen from either Rolf Neugebauer or Andrew Tolmach. 
    // Probably Rolf.

    // double a = 0.0;
    // int e = 0;
    // int c;
    // while ((c = *s++) != '\0' && (c >= '0' && c <= '9')) {
    //     a = a*10.0 + (c - '0');
    // }
    // if (c == '.') {
    //     while ((c = *s++) != '\0' && (c >= '0' && c <= '9')) {
    //         a = a*10.0 + (c - '0');
    //         e = e-1;
    //     }
    // }
    // if (c == 'e' || c == 'E') {
    //     int sign = 1;
    //     int i = 0;
    //     c = *s++;
    //     if (c == '+')
    //         c = *s++;
    //     else if (c == '-') {
    //         c = *s++;
    //         sign = -1;
    //     }
    //     while ((c >= '0' && c <= '9')) {
    //         i = i*10 + (c - '0');
    //         c = *s++;
    //     }
    //     e += i*sign;
    // }
    // while (e > 0) {
    //     a *= 10.0;
    //     e--;
    // }
    // while (e < 0) {
    //     a *= 0.1;
    //     e++;
    // }
    // return a;
}

// INTERNAL FUNCS, DO NOT MOVE AROUND

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
