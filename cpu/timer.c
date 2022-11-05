#include <cpu/ports.h>
#include <cpu/timer.h>
#include <function.h>
#include <cpu/isr.h>

uint32_t tick = 0;

static void timer_callback(registers_t *regs) {
    UNUSED(regs);
    tick++;
}

uint32_t timer_get_tick() {
    return tick;
}

void timer_init(uint32_t freq) {
    if (tick > 1000) tick = 0;
    // Install the function we just wrote
    register_interrupt_handler(IRQ0, timer_callback);

    // Get the PIT value: hardware clock at 1193180 Hz
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    // Send the command
    port_byte_out(0x43, 0x36); // Command port
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}
