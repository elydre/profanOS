#include <filesys.h>
#include <syscall.h>
#include <profan.h>
#include <libmmq.h>
#include <panda.h>

void init_devio(void);
int keyboard_read(void *buffer, uint32_t size, char *term);

int main(void) {
    init_devio();
    return 0;
}

int dev_null(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return 0;
    return size;
}

int dev_zero(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        memset(buffer, 0, size);
    return size;
}

int dev_rand(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (!is_read)
        return 0;
    for (uint32_t i = 0; i < size; i++)
        ((uint8_t *) buffer)[i] = (uint8_t) (rand() & 0xFF);
    return size;
}

int dev_kterm(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return keyboard_read(buffer, size, "/dev/kterm");
    c_kprint((char *) buffer);
    return size;
}

int dev_panda(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return keyboard_read(buffer, size, "/dev/panda");
    panda_print_string((char *) buffer, size);
    return size;
}

int dev_userial(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (!is_read) {
        for (uint32_t i = 0; i < size; i++) {
            if (((char *) buffer)[i] == '\n')
                c_serial_write(SERIAL_PORT_A, "\r", 1);
            c_serial_write(SERIAL_PORT_A, (char *) buffer + i, 1);
        }
        return size;
    }

    if (buffer_addr == NULL) {
        buffer_addr = open_input_serial(NULL, SERIAL_PORT_A);
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

int dev_serial_a(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        c_serial_read(SERIAL_PORT_A, buffer, size);
    else
        c_serial_write(SERIAL_PORT_A, (char *) buffer, size);
    return size;
}

int dev_serial_b(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        c_serial_read(SERIAL_PORT_B, buffer, size);
    else
        c_serial_write(SERIAL_PORT_B, (char *) buffer, size);
    return size;
}

int dev_stdin(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return fm_read(0, buffer, size);
    return 0;
}

int dev_stdout(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return 0;
    return fm_write(1, buffer, size);
}

int dev_stderr(void *buffer, uint32_t offset, uint32_t size, uint8_t is_read) {
    if (is_read)
        return 0;
    return fm_write(2, buffer, size);
}

void init_devio(void) {
    fu_fctf_create(0, "/dev/null",   dev_null);
    fu_fctf_create(0, "/dev/zero",   dev_zero);
    fu_fctf_create(0, "/dev/random", dev_rand);

    fu_fctf_create(0, "/dev/kterm",   dev_kterm);
    fu_fctf_create(0, "/dev/panda",   dev_panda);
    fu_fctf_create(0, "/dev/userial", dev_userial);
    fu_fctf_create(0, "/dev/serialA", dev_serial_a);
    fu_fctf_create(0, "/dev/serialB", dev_serial_b);

    fu_fctf_create(0, "/dev/stdin",  dev_stdin);
    fu_fctf_create(0, "/dev/stdout", dev_stdout);
    fu_fctf_create(0, "/dev/stderr", dev_stderr);
}

int keyboard_read(void *buffer, uint32_t size, char *term) {
    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (buffer_addr == NULL) {
        buffer_addr = open_input_keyboard(NULL, term);
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
