#include <driver/rtc.h>
#include <filesystem.h>
#include <function.h>
#include <cpu/isr.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <task.h>

void kernel_main() {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    isr_install();
    irq_install();
    fskprint("$7ISR initialized\n");
    
    tasking_init();
    fskprint("$7Tasking initialized\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("$7RTC initialized\n");

    filesystem_init();
    fskprint("$7FileSys initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    // launch of the shell.bin
    sys_run_ifexist("/bin/shell.bin", 0);
    start_kshell();
}
