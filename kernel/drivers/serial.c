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

#include <kernel/process.h>
#include <drivers/serial.h>
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

int serial_init(void) {
    serial_enable(SERIAL_PORT_A);
    serial_enable(SERIAL_PORT_B);
    return 0;
}

void serial_send(int device, char out) {
    while (!(port_byte_in(device + 5) & 0x20));
    port_byte_out(device, out);
}

char serial_recv(int device) {
    while (!(port_byte_in(device + 5) & 1))
        process_sleep(process_get_pid(), 1);
    return port_byte_in(device);
}

int serial_write(int device, char *buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        serial_send(device, buf[i]);
    return len;
}

int serial_read(int device, char *buf, uint32_t len) {
    for (uint32_t i = 0; i < len; i++)
        buf[i] = serial_recv(device);
    return len;
}
