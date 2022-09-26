#include <driver/screen.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <filesystem.h>
#include <function.h>
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
    
    tasking_init();
    kprint("Tasking initialized\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    kprint("RTC initialized\n");

    filesystem_init();
    kprint("FileSys initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n", VERSION);
    shell_run();
}
