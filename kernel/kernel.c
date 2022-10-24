#include <driver/serial.h>
#include <driver/rtc.h>
#include <filesystem.h>
#include <function.h>
#include <cpu/isr.h>
#include <ramdisk.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <task.h>
#include <mem.h>

void kernel_main() {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    isr_install();
    irq_install();
    fskprint("ISR initialized\n");

    tasking_init();
    fskprint("tasking initialized\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("RTC initialized\n");

    serial_init();
    fskprint("serial initialized\n");

    ramdisk_init();
    filesystem_init();
    fskprint("filesys initialized\n");

    init_watfunc();
    fskprint("watfunc initialized\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", VERSION);

    // launch of the shell.bin
    char *argv[1];
    argv[0] = "/bin/shell.bin";
    run_ifexist(argv[0], 1, argv);

    task_menu();

    sys_fatal("Nothing to run!");
}
