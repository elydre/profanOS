/*****************************************************************************\
|   === devio.c : 2024 ===                                                    |
|                                                                             |
|    Implementation of /dev devices in a kernel module             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#define _SYSCALL_CREATE_STATIC
#include <profan/syscall.h>

#include <profan/filesys.h>
#include <profan/libmmq.h>
#include <profan/panda.h>
#include <profan.h>

#undef UNUSED
#define UNUSED(x) (void) (x)

int init_devio(void);

int main(void) {
    if (init_devio()) {
        syscall_kprint("[devio] Failed to initialize devices\n");
        return 1;
    }
    return 0;
}

static int keyboard_read(void *buffer, uint32_t size, char *term) {
    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (buffer_addr == NULL) {
        buffer_addr = profan_input_keyboard(NULL, term);
        already_read = 0;
    }

    uint32_t to_read = size;
    uint32_t buffer_size = str_len(buffer_addr);

    if (already_read + to_read > buffer_size) {
        to_read = buffer_size - already_read;
    }

    mem_cpy(buffer, buffer_addr + already_read, to_read);
    already_read += to_read;

    if (already_read >= buffer_size) {
        kfree(buffer_addr);
        buffer_addr = NULL;
    }

    return to_read;
}


int dev_null(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(buffer);
    UNUSED(id);

    return (mode == FCTF_WRITE) ? size : 0;
}

int dev_zero(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    switch (mode) {
        case FCTF_READ:
            mem_set(buffer, 0, size);
            return size;
        case FCTF_WRITE:
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_rand(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    static uint32_t rand_seed = 0;

    if (mode != FCTF_READ)
        return 0;

    for (uint32_t i = 0; i < size; i++) {
        rand_seed = rand_seed * 1103515245 + 12345;
        ((uint8_t *) buffer)[i] = (uint8_t) (rand_seed / 65536) % 256;
    }

    return size;
}

int dev_kterm(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    switch (mode) {
        case FCTF_READ:
            return keyboard_read(buffer, size, "/dev/kterm");
        case FCTF_WRITE:
            syscall_kcnprint((char *) buffer, size, 0x0F);
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_panda(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    switch (mode) {
        case FCTF_READ:
            return keyboard_read(buffer, size, "/dev/panda");
        case FCTF_WRITE:
            panda_print_string((char *) buffer, size, -1, 0x0F);
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_pander(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    static uint8_t color = 0x0C;

    switch (mode) {
        case FCTF_READ:
            return keyboard_read(buffer, size, "/dev/pander");
        case FCTF_WRITE:
            color = panda_print_string((char *) buffer, size, color, 0x0C);
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_userial(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    static char *buffer_addr = NULL;
    static uint32_t already_read = 0;

    if (mode == FCTF_WRITE) {
        for (uint32_t i = 0; i < size; i++) {
            if (((char *) buffer)[i] == '\n')
                syscall_serial_write(SERIAL_PORT_A, "\r", 1);
            syscall_serial_write(SERIAL_PORT_A, (char *) buffer + i, 1);
        }
        return size;
    }

    if (mode != FCTF_READ)
        return 0;

    if (buffer_addr == NULL) {
        buffer_addr = profan_input_serial(NULL, SERIAL_PORT_A);
        already_read = 0;
    }

    uint32_t to_read = size;
    uint32_t buffer_size = str_len(buffer_addr);

    if (already_read + to_read > buffer_size) {
        to_read = buffer_size - already_read;
    }

    mem_cpy(buffer, buffer_addr + already_read, to_read);
    already_read += to_read;

    if (already_read >= buffer_size) {
        kfree(buffer_addr);
        buffer_addr = NULL;
    }

    return to_read;
}

int dev_serial_a(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    switch (mode) {
        case FCTF_READ:
            syscall_serial_read(SERIAL_PORT_A, buffer, size);
            return size;
        case FCTF_WRITE:
            syscall_serial_write(SERIAL_PORT_A, (char *) buffer, size);
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_serial_b(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    switch (mode) {
        case FCTF_READ:
            syscall_serial_read(SERIAL_PORT_B, buffer, size);
            return size;
        case FCTF_WRITE:
            syscall_serial_write(SERIAL_PORT_B, (char *) buffer, size);
            return size;
        default:
            return 0;
    }

    return 0;
}

int dev_stdin(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    if (mode == FCTF_READ)
        return fm_read(0, buffer, size);

    return 0;
}

int dev_stdout(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    if (mode == FCTF_WRITE)
        return fm_write(1, buffer, size);

    return 0;
}

int dev_stderr(int id, void *buffer, uint32_t size, uint8_t mode) {
    UNUSED(id);

    if (mode == FCTF_WRITE)
        return fm_write(2, buffer, size);

    return 0;
}

int init_devio(void) {
    return (
        IS_SID_NULL(fu_fctf_create(0, "/dev/null",   dev_null))      ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/zero",   dev_zero))      ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/random", dev_rand))      ||

        IS_SID_NULL(fu_fctf_create(0, "/dev/kterm",   dev_kterm))    ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/panda",   dev_panda))    ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/pander",  dev_pander))   ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/userial", dev_userial))  ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/serialA", dev_serial_a)) ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/serialB", dev_serial_b)) ||

        IS_SID_NULL(fu_fctf_create(0, "/dev/stdin",  dev_stdin))     ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/stdout", dev_stdout))    ||
        IS_SID_NULL(fu_fctf_create(0, "/dev/stderr", dev_stderr))    ||

        IS_SID_NULL(fu_file_create(0, "/dev/clip"))
    );
}
