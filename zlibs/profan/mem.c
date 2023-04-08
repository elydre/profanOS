#include <syscall.h>

int main() {
    return 0;
}

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
    int size = c_mem_get_alloc_size((uint32_t) addr);
    if (size == 0) return;
    mem_set((uint8_t *) addr, 0, size);
    c_mem_free_addr((int) addr);
}

void *malloc(uint32_t size) {
    uint32_t addr = c_mem_alloc(size, 1);
    if (addr == 0) return NULL; // error
    return (void *) addr;
}

void *realloc(void *ptr, uint32_t size) {
    uint32_t addr = (uint32_t) ptr;
    uint32_t new_addr = c_mem_alloc(size, 1);
    if (new_addr == 0) return NULL;
    mem_copy((uint8_t *) addr, (uint8_t *) new_addr, size);
    c_mem_free_addr(addr);
    return (void *) new_addr;
}

void *calloc(uint32_t size) {
    int addr = c_mem_alloc(size, 1);
    if (addr == 0) return NULL;
    mem_set((uint8_t *) addr, 0, size);
    return (void *) addr;
}
