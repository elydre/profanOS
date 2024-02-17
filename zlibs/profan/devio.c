#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>
#include <panda.h>

#include <filesys.h>

// modes definition
#define MODE_WRITE 0
#define MODE_READD 1

void init_devio();

int main(void) {
    init_devio();
    return 0;
}

int dev_null_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READD) {
        memset(buffer, 0, size);
        return size;
    }
    return 0;
}

int dev_rand_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode != MODE_READD) {
        return 0;
    }

    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *) buffer)[i] = (uint8_t) rand() & 0xFF;
    }

    return size;
}

int dev_kterm_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_kprint((char *) buffer);
        return size;
    }

    return 0;
}

int dev_panda_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        panda_print_string((char *) buffer, size);
        return size;
    } else if (mode == MODE_READD) {
        if (size < 2 * sizeof(uint32_t)) {
            return 0;
        }
        panda_get_cursor((uint32_t *) buffer, (uint32_t *) buffer + 1);
        return 2 * sizeof(uint32_t);
    }

    return 0;
}

int dev_serial_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_serial_print(SERIAL_PORT_A, (char *) buffer);
        return size;
    }

    return 0;
}

int dev_kb_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (mode != MODE_READD) {
        return 0;
    }

    if (buffer_addr == NULL) {
        buffer_addr = open_input(NULL);
        already_read = 0;
    }

    uint32_t to_read = size;
    uint32_t buffer_size = strlen(buffer_addr);

    if (already_read + to_read > buffer_size) {
        to_read = buffer_size - already_read;
    }

    memcpy(buffer, buffer_addr + already_read, to_read);
    already_read += to_read;

    if (already_read >= buffer_size) {
        free(buffer_addr);
        buffer_addr = NULL;
    }

    return to_read;
}

int dev_stdin_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READD) {
        return fm_read(0, buffer, size);
    }
    return 0;
}

int dev_stdout_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        return fm_write(1, buffer, size);
    }
    return 0;
}

int dev_stderr_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        return fm_write(2, buffer, size);
    }
    return 0;
}

void init_devio(void) {
    fu_fctf_create(0, "/dev/null",   dev_null_rw);
    fu_fctf_create(0, "/dev/random", dev_rand_rw);

    fu_fctf_create(0, "/dev/kterm",  dev_kterm_rw);
    fu_fctf_create(0, "/dev/panda",  dev_panda_rw);
    fu_fctf_create(0, "/dev/serial", dev_serial_rw);
    fu_fctf_create(0, "/dev/kb",     dev_kb_rw);

    fu_fctf_create(0, "/dev/stdin",  dev_stdin_rw);
    fu_fctf_create(0, "/dev/stdout", dev_stdout_rw);
    fu_fctf_create(0, "/dev/stderr", dev_stderr_rw);

    setenv("TERM", "/dev/kterm", 1);
}
