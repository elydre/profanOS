/*****************************************************************************\
|   === rtc.c : 2024 ===                                                      |
|                                                                             |
|    Kernel Real Time Clock driver                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <drivers/rtc.h>
#include <cpu/ports.h>

char bcd;

unsigned char read_register(unsigned char reg) {
    port_byte_out(0x70, reg);
    return port_byte_in(0x71);
}

void write_register(unsigned char reg, unsigned char value) {
    port_byte_out(0x70, reg);
    port_byte_out(0x71, value);
}

unsigned char bcd2bin(unsigned char in_bcd) {
    return (bcd) ? ((in_bcd >> 4) * 10) + (in_bcd & 0x0F) : in_bcd;
}

int time_get(tm_t *target) {
    target->tm_sec = bcd2bin(read_register(0x00));
    target->tm_min = bcd2bin(read_register(0x02));
    target->tm_hour = bcd2bin(read_register(0x04));
    target->tm_wday = bcd2bin(read_register(0x06));
    target->tm_mday = bcd2bin(read_register(0x07));
    target->tm_mon = bcd2bin(read_register(0x08));
    target->tm_year = bcd2bin(read_register(0x09));

    target->tm_yday = -1;
    target->tm_isdst = -1;

    return 0;
}

int rtc_init(void) {
    unsigned char status;
    status = read_register(0x0B);
    status |=  0x02;             // 24 hour clock
    status |=  0x10;             // update ended interrupts
    status &= ~0x20;             // no alarm interrupts
    status &= ~0x40;             // no periodic interrupt
    bcd = !(status & 0x04);      // check if data type is BCD
    write_register(0x0B, status);
    return 0;
}
