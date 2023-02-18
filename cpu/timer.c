#include <kernel/process.h>
#include <cpu/ports.h>
#include <cpu/timer.h>
#include <cpu/isr.h>
#include <system.h>


#define SCHEDULER_EVRY (RATE_TIMER_TICK / RATE_SCHEDULER)

uint32_t tick = 0;

static void timer_callback(registers_t *regs) {
    (void) regs;

    tick++;

    /*if (tick % SCHEDULER_EVRY == 0) {
        schedule();
    }*/

#if RATE_COSMIC_RAY != 0
    sys_cosmic_ray();
#endif
}

uint32_t timer_get_tick() {
    return tick;
}

uint32_t timer_get_ms() {
    return tick * 1000 / RATE_TIMER_TICK;
}

int timer_init() {
    if (tick > 1000) tick = 0;
    // Install the function we just wrote
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
