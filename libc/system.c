#include <system.h>
#include <ports.h>


void sys_reboot() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    asm volatile("hlt");
}

void sys_shutdown() {
    asm volatile("hlt");
}
