#include <libc/filesystem.h>
#include <libc/multiboot.h>
#include <driver/serial.h>
#include <libc/ramdisk.h>
#include <driver/rtc.h>
#include <libc/task.h>
#include <function.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <cpu/gdt.h>
#include <system.h>
#include <iolib.h>
#include <time.h>
#include <type.h>
#include <mem.h>

void kernel_main(void *mboot_ptr) {
    clear_screen();
    fskprint("$6booting profanOS...\n");

    mboot_save(mboot_ptr);
    fskprint("Mboot saved\n");

    gdt_init();
    fskprint("GDT init\n");

    isr_install();
    fskprint("ISR init\n");
    
    irq_install();
    fskprint("IRQ init\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    fskprint("RTC init\n");

    serial_init();
    fskprint("serial init\n");

    init_vesa();
    fskprint("vesa init\n");

    tasking_init();
    fskprint("tasking init\n");

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
    argv[0] = "/bin/3drenderer.bin";
    run_ifexist(argv[0], 1, argv);

    task_menu();

    sys_fatal("Nothing to run!");
}
