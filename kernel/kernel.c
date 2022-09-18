#include <driver/screen.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <cpu/isr.h>
#include "kernel.h"
#include <iolib.h>
#include <time.h>
#include <task.h>


void kernel_main() {
    clear_screen();
    ckprint("booting profanOS...\n", c_grey);

    isr_install();
    irq_install();
    kprint("ISR initialized\n");
    
    init_tasking();
    kprint("Tasking initialized\n");

    rtc_install();
    gen_boot_time();
    kprint("RTC initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$Cversion $4%s\n\n", VERSION);
    run_shell();
}
