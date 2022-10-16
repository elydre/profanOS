#include <driver/rtc.h>
#include <filesystem.h>
#include <function.h>
#include <cpu/isr.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <task.h>

int find_mem_end() {
    // Find the end of the memory
    // TODO: move in mem.c or system.c
    int *addr_min = (int *) 0x200000;
    int *addr_max = (int *) 0x40000000;
    int *addr_test;
    while (addr_max - addr_min > 1) {
        addr_test = addr_min + (addr_max - addr_min) / 2;
        * addr_test = 0x1234;
        if (*addr_test == 0x1234) addr_min = addr_test;
        else addr_max = addr_test;
    }
    return (int) addr_max;
}

void kernel_main() {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    isr_install();
    irq_install();
    fskprint("ISR initialized\n");
    
    tasking_init();
    fskprint("Tasking initialized\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("RTC initialized\n");

    filesystem_init();
    fskprint("FileSys initialized\n");
        
    init_watfunc();
    fskprint("WatFunc initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    fskprint("Memory end: %dMo\n", find_mem_end() / 1024 / 1024);

    // launch of the shell.bin
    sys_run_ifexist("/bin/shell.bin", 0);
    start_kshell();
    
    sys_fatal("Nothing to run!");
}
