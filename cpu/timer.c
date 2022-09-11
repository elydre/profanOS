#include <cpu/timer.h>
#include <function.h>
#include <cpu/isr.h>
#include <system.h>
#include <ports.h>
#include <time.h>

uint32_t tick = 0;
int last_refresh;
int refresh_time[5];

static void timer_callback(registers_t *regs) {
    UNUSED(regs);
    tick++;
    if (tick % 100 == 0) {
        int now = gen_unix_time();
        for (int i = 5; i > 0; i--) {
            refresh_time[i] = refresh_time[i-1];
        }
        refresh_time[0] = now - last_refresh;
        last_refresh = now;
    }
}

uint32_t timer_get_tick() {
    return tick;
}

void timer_get_refresh_time(int target[5]) {
    for (int i = 0; i < 5; i++) {
        target[i] = refresh_time[i];
    }
}

void timer_sleep(uint32_t ms) {
    uint32_t start_tick = timer_get_tick();
    while (timer_get_tick() < start_tick + ms / 10) {
        do_nothing();
    }
}

void init_timer(uint32_t freq) {
    if (tick > 1000) tick = 0;
    /* Install the function we just wrote */
    register_interrupt_handler(IRQ0, timer_callback);

    /* Get the PIT value: hardware clock at 1193180 Hz */
    uint32_t divisor = 1193180 / freq;
    uint8_t low  = (uint8_t)(divisor & 0xFF);
    uint8_t high = (uint8_t)( (divisor >> 8) & 0xFF);
    /* Send the command */
    port_byte_out(0x43, 0x36); /* Command port */
    port_byte_out(0x40, low);
    port_byte_out(0x40, high);
}
