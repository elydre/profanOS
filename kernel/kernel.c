#include <libc/filesystem.h>
#include <driver/serial.h>
#include <libc/ramdisk.h>
#include <driver/rtc.h>
#include <libc/task.h>
#include <function.h>
#include <cpu/isr.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <mem.h>

void kernel_main() {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    isr_install();
    irq_install();
    fskprint("ISR init\n");

    tasking_init();
    fskprint("tasking init\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("RTC init\n");

    serial_init();
    fskprint("serial init\n");

    ramdisk_init();
    fskprint("ramdisk init\n");
    
    filesystem_init();
    fskprint("filesys init\n");

    init_watfunc();
    fskprint("watfunc init\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    // launch of the shell.bin
    char *argv[1];
    argv[0] = "/bin/shell.bin";
    run_ifexist(argv[0], 1, argv);

    task_menu();

    sys_fatal("Nothing to run!");
}
