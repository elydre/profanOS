/****** This file is part of profanOS **************************\
|   == ports.c ==                                    .pi0iq.    |
|                                                   d"  . `'b   |
|   Kernel Input/Output ports functions             q. /|\  u   |
|                                                    `// \\     |
|                                                    //   \\    |
|   [ github.com/elydre/profanOS - GPLv3 ]          //     \\   |
\***************************************************************/

#include <cpu/ports.h>

#include <ktype.h>

// Read a byte from the specified port
uint8_t port_byte_in(uint16_t port) {
    uint8_t result;
    asm("in %%dx, %%al" : "=a" (result) : "d" (port));
    return result;
}

void port_byte_out(uint16_t port, uint8_t data) {
    asm volatile("out %%al, %%dx" : : "a" (data), "d" (port));
}

uint16_t port_word_in(uint16_t port) {
    uint16_t result;
    asm("in %%dx, %%ax" : "=a" (result) : "d" (port));
    return result;
}

void port_word_out(uint16_t port, uint16_t data) {
    asm volatile("out %%ax, %%dx" : : "a" (data), "d" (port));
}

uint32_t port_long_in(uint32_t port) {
    uint32_t result;
    asm volatile("inl %%dx,%%eax":"=a" (result):"d"(port));
    return result;
}

void port_long_out(uint32_t port, uint32_t value) {
    asm volatile("outl %%eax,%%dx"::"d" (port), "a" (value));
}
