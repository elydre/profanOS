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

#include <kernel/butterfly.h>
#include <kernel/afft.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

#include <profan/filesys.h>
#include <profan.h>

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

    mem_copy(buffer, buffer_addr + already_read, to_read);
    already_read += to_read;

    if (already_read >= buffer_size) {
        free(buffer_addr);
        buffer_addr = NULL;
    }

    return to_read;
}

int dev_null_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(buffer);
    UNUSED(offset);

    return 0;
}

int dev_null_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(buffer);
    UNUSED(offset);

    return size;
}

int dev_zero_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    mem_set(buffer, 0, size);
    return size;
}

int dev_rand_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    static uint32_t rand_seed = 0;

    for (uint32_t i = 0; i < size; i++) {
        rand_seed = rand_seed * 1103515245 + 12345;
        ((uint8_t *) buffer)[i] = (uint8_t) (rand_seed / 65536) % 256;
    }

    return size;
}

int dev_kterm_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    return keyboard_read(buffer, size, "/dev/kterm");
}

int dev_kterm_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    kcnprint((char *) buffer, size, 0x0F);

    return size;
}

int dev_stdin_r(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    return fm_read(0, buffer, size);
}

int dev_stdout_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    return fm_write(1, buffer, size);
}

int dev_stderr_w(void *buffer, uint32_t offset, uint32_t size) {
    UNUSED(offset);

    return fm_write(2, buffer, size);
}

static int setup_afft(const char *name, void *read, void *write, void *cmd) {
    int afft_id = afft_register(AFFT_AUTO, read, write, cmd);

    return (afft_id == -1 || kfu_afft_create("/dev", name, afft_id) == SID_NULL);
}

int __init(void) {
    if (
        setup_afft("null", dev_null_r, dev_null_w, NULL) ||
        setup_afft("zero", dev_zero_r, dev_null_w, NULL) ||
        setup_afft("rand", dev_rand_r, NULL, NULL) ||
        setup_afft("kterm", dev_kterm_r, dev_kterm_w, NULL) ||
        setup_afft("stdin", dev_stdin_r, NULL, NULL) ||
        setup_afft("stdout", NULL, dev_stdout_w, NULL) ||
        setup_afft("stderr", NULL, dev_stderr_w, NULL)
    ) {
        kprint("[devio] Failed to initialize devices\n");
        return 1;
    }

    return 0;
}
