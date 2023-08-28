#include <kernel/process.h>
#include <cpu/ports.h>
#include <cpu/timer.h>
#include <cpu/isr.h>
#include <system.h>

uint32_t ticks = 0;

static void timer_callback(registers_t *regs) {
    (void) regs;

    ticks++;

    schedule(ticks);
}

uint32_t timer_get_ticks() {
    return ticks;
}

uint32_t timer_get_ms() {
    return ticks * 1000 / RATE_TIMER_TICK;
}

int timer_init() {
    if (ticks > 1000) ticks = 0;

    // set the timer interrupt handler
    register_interrupt_handler(IRQ0, timer_callback);

    // Get the PIT value: hardware clock at 1193180 Hz
    uint32_t divisor = 1193180 / RATE_TIMER_TICK;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    // Send the command
    port_byte_out(0x43, 0x36); // Command port
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);

    return 0;
}
