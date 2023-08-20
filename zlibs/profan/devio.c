#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

void init_devio();

int main(void) {
    init_devio();
    return 0;
}

int devzero_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        memset(buffer, 0, size);

    return 0;
}

int devnull_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    return 0;
}

int devrand_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (!is_read)
        return 0;

    for (uint32_t i = 0; i < size; i++)
        ((uint8_t *) buffer)[i] = (uint8_t) rand();

    return 0;
}

void init_devio() {
    fu_fctf_create(0, "/dev/zero", devzero_rw);
    fu_fctf_create(0, "/dev/null", devnull_rw);
    fu_fctf_create(0, "/dev/random", devrand_rw);
}
