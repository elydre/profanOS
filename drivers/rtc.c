#include "rtc.h"
#include "../cpu/ports.h"
#include "../libc/string.h"
#include "screen.h"


int bcd;


unsigned char read_register(unsigned char reg) {
    outportb(0x70, reg);
    return inportb(0x71);
}

void write_register(unsigned char reg, unsigned char value) {
    outportb(0x70, reg);
    outportb(0x71, value);
}

unsigned char bcd2bin(unsigned char bcd) {
    return ((bcd >> 4) * 10) + (bcd & 0x0F);
}

void get_time(time_t *target) {
    if (bcd){
        target->seconds = bcd2bin(read_register(0x00));
        target->minutes = bcd2bin(read_register(0x02));
        target->hours = bcd2bin(read_register(0x04));
        target->day_of_week = bcd2bin(read_register(0x06));
        target->day_of_month = bcd2bin(read_register(0x07));
        target->month = bcd2bin(read_register(0x08));
        target->year = bcd2bin(read_register(0x09));
    } else {
        target->seconds = read_register(0x00);
        target->minutes = read_register(0x02);
        target->hours = read_register(0x04);
        target->day_of_week = read_register(0x06);
        target->day_of_month = read_register(0x07);
        target->month = read_register(0x08);
        target->year = read_register(0x09);
    }

    target->full[0] = target->seconds;
    target->full[1] = target->minutes;
    target->full[2] = target->hours;
    target->full[3] = target->day_of_month;
    target->full[4] = target->month;
    target->full[5] = target->year;
}

void rtc_install(void) {
    unsigned char status;
    status = read_register(0x0B);
    status |=  0x02;             // 24 hour clock
    status |=  0x10;             // update ended interrupts
    status &= ~0x20;             // no alarm interrupts
    status &= ~0x40;             // no periodic interrupt
    bcd  =  !(status & 0x04);    // check if data type is BCD
    write_register(0x0B, status);
}
