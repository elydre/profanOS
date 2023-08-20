#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#define STDOUT_BUFFER_SIZE 1024

typedef struct {
    uint8_t buffer[STDOUT_BUFFER_SIZE];
    uint32_t offset;
} stdout_buffer_t;

stdout_buffer_t *stdout_buffer;

#define MODE_READ  0
#define MODE_WRITE 1
#define MODE_FLUSH 2

void init_devio();

int main(void) {
    stdout_buffer = malloc(sizeof(stdout_buffer_t));
    init_devio();
    return 0;
}

int devzero_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READ) {
        memset(buffer, 0, size);
        return size;
    }
    return 0;
}

int devnull_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    return 0;
}

int devrand_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode != MODE_READ) {
        return 0;
    }

    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *) buffer)[i] = (uint8_t) rand();
    }

    return size;
}

int devzebra_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_ckprint((char *) buffer, c_dgreen);
        return size;
    }

    return 0;
}

int devstdout_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (offset) {
        printf("offset is not supported for stdout\n");
        return 0;
    }

    if (mode == MODE_WRITE) {
        for (int i = 0; i < size; i++) {
            stdout_buffer->buffer[stdout_buffer->offset++] = ((uint8_t *) buffer)[i];
            if (stdout_buffer->offset >= STDOUT_BUFFER_SIZE) {
                devzebra_rw(stdout_buffer->buffer, 0, STDOUT_BUFFER_SIZE, MODE_WRITE);
                stdout_buffer->offset = 0;
            }
            else if (((uint8_t *) buffer)[i] == '\n') {
                devzebra_rw(stdout_buffer->buffer, 0, stdout_buffer->offset, MODE_WRITE);
                stdout_buffer->offset = 0;
            }
        }
    } else if (mode == MODE_FLUSH) {
        devzebra_rw(stdout_buffer->buffer, 0, stdout_buffer->offset, MODE_WRITE);
        stdout_buffer->offset = 0;
    }

    return 0;
}

void init_devio() {
    fu_fctf_create(0, "/dev/zero", devzero_rw);
    fu_fctf_create(0, "/dev/null", devnull_rw);
    fu_fctf_create(0, "/dev/random", devrand_rw);
    fu_fctf_create(0, "/dev/zebra", devzebra_rw);
    fu_fctf_create(0, "/dev/stdout", devstdout_rw);
}
