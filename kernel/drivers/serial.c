/*****************************************************************************\
|   === serial.c : 2024 ===                                                   |
|                                                                             |
|    Kernel Serial driver                                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <kernel/afft.h>
#include <cpu/ports.h>
#include <minilib.h>
#include <ktype.h>

/********************************
 *                             *
 *   serial driver functions   *
 *                             *
********************************/

void serial_enable(int device) {
    port_write8(device + 1, 0x00);
    port_write8(device + 3, 0x80); // enable divisor mode
    port_write8(device + 0, 0x03); // div Low:  03 Set the port to 38400 bps
    port_write8(device + 1, 0x00); // div High: 00
    port_write8(device + 3, 0x03);
    port_write8(device + 2, 0xC7);
    port_write8(device + 4, 0x0B);
}

int serial_write(uint32_t port, void *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!(port_read8(port + 5) & 0x20))
            process_sleep(process_get_pid(), 1);
        port_write8(port, ((char *) buffer)[i]);
    }
    return len;
}

int serial_read(uint32_t port, void *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!(port_read8(port + 5) & 1))
            process_sleep(process_get_pid(), 1);
        ((char *) buffer)[i] = port_read8(port);
    }
    return len;
}

/********************************
 *                             *
 *  AFFT read/write handlers   *
 *                             *
********************************/

#define SERIAL_AFFT_A 1
#define SERIAL_AFFT_B 2

static int serial_afft_write(uint32_t id, void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);
    
    switch (id) {
        case SERIAL_AFFT_A:
            return serial_write(SERIAL_PORT_A, buffer, len);
        case SERIAL_AFFT_B:
            return serial_write(SERIAL_PORT_B, buffer, len);
        default:
            return -1;
    }
}

static int serial_afft_read(uint32_t id, void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);

    switch (id) {
        case SERIAL_AFFT_A:
            return serial_read(SERIAL_PORT_A, buffer, len);
        case SERIAL_AFFT_B:
            return serial_read(SERIAL_PORT_B, buffer, len);
        default:
            return -1;
    }
}

int serial_init(void) {

    serial_enable(SERIAL_PORT_A);
    serial_enable(SERIAL_PORT_B);

    return (
        afft_register(SERIAL_AFFT_A, serial_afft_read, serial_afft_write, NULL) != SERIAL_AFFT_A ||
        afft_register(SERIAL_AFFT_B, serial_afft_read, serial_afft_write, NULL) != SERIAL_AFFT_B ||
        kfu_afft_create("/dev", "serialA", SERIAL_AFFT_A) == SID_NULL ||
        kfu_afft_create("/dev", "serialB", SERIAL_AFFT_B) == SID_NULL
    );
}
