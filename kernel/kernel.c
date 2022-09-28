#include <driver/screen.h>
#include <kernel/shell.h>
#include <driver/rtc.h>
#include <filesystem.h>
#include <cpu/timer.h>
#include <function.h>
#include <cpu/isr.h>
#include "kernel.h"
#include <system.h>
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
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    // launch of the shell :
    char shell_path[] = "/bin/shell.bin";
    if (fs_does_path_exists(shell_path)) {
        sys_run_binary(shell_path, 0);
    } else  {
        sys_fatal(42, "Shell not found, exiting...\n");
    }
    shell_run();
}
