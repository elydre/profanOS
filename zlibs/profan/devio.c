#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <i_iolib.h>
#include <panda.h>

#define DEVIO_LIB_C
#include <filesys.h>

// lc definition
#define LCS_COUNT 3
#define LCS_SIZE  1024

typedef struct {
    uint8_t  buffer[LCS_SIZE];
    uint32_t offset;

    sid_t    redirection;
    uint32_t redirection_offset;
} lc_t;

lc_t **lc;

// modes definition
#define MODE_WRITE 0
#define MODE_READD 1
#define MODE_FLUSH 2

void init_devio();
void init_lcbuffer();

int main(void) {
    init_devio();
    init_lcbuffer();

    return 0;
}

int devio_change_redirection(uint32_t lc_index, sid_t redirection) {
    if (lc_index >= LCS_COUNT) {
        return 1;
    }

    if (IS_NULL_SID(redirection)) {
        return 1;
    }

    lc[lc_index]->redirection = redirection;
    lc[lc_index]->redirection_offset = 0;
    return 0;
}

int write_in_file(sid_t sid, void *buffer, uint32_t offset, uint32_t size) {
    if (IS_NULL_SID(sid)) {
        return 0;
    }

    if (fu_is_fctf(sid)) {
        fu_fctf_write(sid, buffer, offset, size);
        return size;
    }
    if (fu_is_file(sid)) {
        if (size + offset > fu_get_file_size(sid)) {
            if (fu_set_file_size(sid, size + offset)) return 0;
        }
        if (fu_file_write(sid, buffer, offset, size)) return 0;
        return size;
    }
    printf("no method to write in sid d%ds%d\n", sid.device, sid.sector);
    return 0;
}

void read_in_file(sid_t sid, void *buffer, uint32_t offset, uint32_t size) {
    if (IS_NULL_SID(sid)) {
        return;
    }

    if (fu_is_fctf(sid)) {
        fu_fctf_read(sid, buffer, offset, size);
    } else if (fu_is_file(sid)) {
        fu_file_read(sid, buffer, offset, size);
    } else {
        printf("no method to read in sid d%ds%d\n", sid.device, sid.sector);
    }
}

int devzero_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_READD) {
        memset(buffer, 0, size);
        return size;
    }
    return 0;
}

int devnull_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    return 0;
}

int devrand_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode != MODE_READD) {
        return 0;
    }

    for (uint32_t i = 0; i < size; i++) {
        ((uint8_t *) buffer)[i] = (uint8_t) rand();
    }

    return size;
}

int devzebra_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_kcprint((char *) buffer, c_dgreen);
        return size;
    }

    return 0;
}

int devparrot_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        color_print((char *) buffer);
        return size;
    }

    return 0;
}

int devpanda_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    static char color = c_white;
    if (mode == MODE_WRITE) {
        color = panda_color_print((char *) buffer, color, size);
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

int devserial_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        c_serial_print(SERIAL_PORT_A, (char *) buffer);
        return size;
    }

    return 0;
}

int genbuffer_rw(lc_t *lcptr, void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (offset) {
        printf("offset is not supported by buffer system\n");
        return 0;
    }

    if (mode == MODE_WRITE) {
        for (uint32_t i = 0; i < size; i++) {
            lcptr->buffer[lcptr->offset++] = ((uint8_t *) buffer)[i];
            if (!(lcptr->offset >= LCS_SIZE || ((uint8_t *) buffer)[i] == '\n'))
                continue;

            lcptr->buffer[lcptr->offset] = '\0';
            lcptr->redirection_offset += write_in_file(lcptr->redirection, lcptr->buffer, lcptr->redirection_offset, lcptr->offset);

            lcptr->offset = 0;
        }
    } else if (mode == MODE_FLUSH && lcptr->offset) {
        lcptr->buffer[lcptr->offset] = '\0';
        lcptr->redirection_offset += write_in_file(
                lcptr->redirection,
                lcptr->buffer, 
                lcptr->redirection_offset,
                lcptr->offset
        );
        if (fu_is_fctf(lcptr->redirection)) {
            fu_fctf_flush(lcptr->redirection);
        }
        lcptr->offset = 0;
    } else if (mode == MODE_READD) {
        read_in_file(lcptr->redirection, buffer, 0, size);
    }

    return 0;
}

int devstdout_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    return genbuffer_rw(lc[DEVIO_STDOUT], buffer, offset, size, mode);
}

int devstderr_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    return genbuffer_rw(lc[DEVIO_STDERR], buffer, offset, size, mode);
}

int devbuffer_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    return genbuffer_rw(lc[DEVIO_BUFFER], buffer, offset, size, mode);
}

int devnocolor_rw(void *buffer, uint32_t offset, uint32_t size, uint8_t mode) {
    if (mode == MODE_WRITE) {
        char *copy = strdup((char *) buffer);
        for (uint32_t i = 0; copy[i]; i++) {
            if (copy[i] == '$' && copy[i + 1]) {
                for (uint32_t j = i; copy[j]; j++) {
                    copy[j] = copy[j + 2];
                }
            }
        }
        // push it to buffer
        devbuffer_rw(copy, 0, strlen(copy), MODE_WRITE);
        free(copy);
        return size;
    } else if (mode == MODE_FLUSH) {
        devbuffer_rw(NULL, 0, 0, MODE_FLUSH);
    }

    return 0;
}

void init_devio() {
    fu_fctf_create(0, "/dev/zero",   devzero_rw);
    fu_fctf_create(0, "/dev/null",   devnull_rw);
    fu_fctf_create(0, "/dev/random", devrand_rw);
    fu_fctf_create(0, "/dev/nocolor", devnocolor_rw);

    fu_fctf_create(0, "/dev/zebra",  devzebra_rw);
    fu_fctf_create(0, "/dev/parrot", devparrot_rw);
    fu_fctf_create(0, "/dev/panda",  devpanda_rw);
    fu_fctf_create(0, "/dev/serial", devserial_rw);

    fu_fctf_create(0, "/dev/stdout", devstdout_rw);
    fu_fctf_create(0, "/dev/stderr", devstderr_rw);
    fu_fctf_create(0, "/dev/buffer", devbuffer_rw);
}

void init_lcbuffer() {
    lc = malloc((sizeof(lc_t *) * LCS_COUNT) + (sizeof(lc_t) * LCS_COUNT));
    if (lc == NULL) {
        printf("error while allocating memory for lc\n");
        return;
    }

    for (uint32_t i = 0; i < LCS_COUNT; i++) {
        lc[i] = (lc_t *) ((uint32_t) lc + (sizeof(lc_t *) * LCS_COUNT) + (sizeof(lc_t) * i));
        lc[i]->offset = 0;
    }

    if (
        devio_change_redirection(DEVIO_STDOUT, fu_path_to_sid(ROOT_SID, "/dev/parrot")) ||
        devio_change_redirection(DEVIO_STDERR, fu_path_to_sid(ROOT_SID, "/dev/parrot")) ||
        devio_change_redirection(DEVIO_BUFFER, fu_path_to_sid(ROOT_SID, "/dev/null"))
    ) printf("error while initializing redirections\n");
}
