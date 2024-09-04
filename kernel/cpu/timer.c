/*****************************************************************************\
|   === timer.c : 2024 ===                                                    |
|                                                                             |
|    Kernel Timer (PIT) management                                 .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <kernel/process.h>
#include <cpu/ports.h>
#include <cpu/timer.h>
#include <cpu/isr.h>
#include <system.h>

uint32_t ticks;

static void timer_callback(registers_t *regs) {
    (void) regs;

    ticks++;

    schedule(ticks);
}

uint32_t timer_get_ticks(void) {
    return ticks;
}

uint32_t timer_get_ms(void) {
    return ticks * 1000 / RATE_TIMER_TICK;
}

int timer_init(void) {
    ticks = 1;

    // set the timer interrupt handler
    register_interrupt_handler(IRQ0, timer_callback);

    // get the PIT value: hardware clock at 1193180 Hz
    uint32_t divisor = 1193180 / RATE_TIMER_TICK;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)((divisor >> 8) & 0xFF);
    // send the command
    port_byte_out(0x43, 0x36); // command port
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);

    return 0;
}

