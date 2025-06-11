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


void serial_enable(int device) {
    port_byte_out(device + 1, 0x00);
    port_byte_out(device + 3, 0x80); // enable divisor mode
    port_byte_out(device + 0, 0x03); // div Low:  03 Set the port to 38400 bps
    port_byte_out(device + 1, 0x00); // div High: 00
    port_byte_out(device + 3, 0x03);
    port_byte_out(device + 2, 0xC7);
    port_byte_out(device + 4, 0x0B);
}

int serial_write(uint32_t port, void *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!(port_byte_in(port + 5) & 0x20))
            process_sleep(process_get_pid(), 1);
        port_byte_out(port, ((char *) buffer)[i]);
    }
    return len;
}

int serial_read(uint32_t port, void *buffer, uint32_t len) {
    for (uint32_t i = 0; i < len; i++) {
        while (!(port_byte_in(port + 5) & 1))
            process_sleep(process_get_pid(), 1);
        ((char *) buffer)[i] = port_byte_in(port);
    }
    return len;
}

static int serial_a_write(void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);
    return serial_write(SERIAL_PORT_A, buffer, len);
}

static int serial_a_read(void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);
    return serial_read(SERIAL_PORT_A, buffer, len);
}

static int serial_b_write(void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);
    return serial_write(SERIAL_PORT_B, buffer, len);
}

static int serial_b_read(void *buffer, uint32_t offset, uint32_t len) {
    UNUSED(offset);
    return serial_read(SERIAL_PORT_B, buffer, len);
}

int serial_init(void) {

    serial_enable(SERIAL_PORT_A);
    serial_enable(SERIAL_PORT_B);

    return (
        afft_register(1, serial_a_read, serial_a_write, NULL) != 1   ||
        afft_register(2, serial_b_read, serial_b_write, NULL) != 2   ||
        kfu_afft_create("/dev", "serialA", 1) == SID_NULL ||
        kfu_afft_create("/dev", "serialB", 2) == SID_NULL
    );
}
