#include <syscall.h>

void init_func();

void mem_copy(uint8_t *source, uint8_t *dest, int nbytes);
void mem_set(uint8_t *dest, uint8_t val, uint32_t len);
void mem_move(uint8_t *source, uint8_t *dest, int nbytes);

int main() {
    init_func();
    return 0;
}

void init_func() {
    c_kprint("Init of the stdlib !");
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


long int a64l (const char *string) {
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
