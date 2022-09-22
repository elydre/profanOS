#include <system.h>
#include <ports.h>
#include <iolib.h>


void sys_reboot() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    asm volatile("hlt");
}

void sys_shutdown() {
    port_word_out(0x604, 0x2000);   // qemu
    port_word_out(0xB004, 0x2000);  // bochs
    port_word_out(0x4004, 0x3400);  // virtualbox
    sys_stop();                     // halt if above didn't work
}

void sys_stop() {
    fskprint("$3profanOS has been stopped $5:$3(\n");
    asm volatile("cli");
    asm volatile("hlt");
}

void do_nothing() {
    asm volatile("sti");
    asm volatile("hlt");
    asm volatile("cli");
}
