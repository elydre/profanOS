#include <libc/filesystem.h>
#include <libc/multiboot.h>
#include <driver/serial.h>
#include <libc/ramdisk.h>
#include <driver/rtc.h>
#include <gui/gnrtx.h>
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
    ckprint("booting profanOS...\n", 0x07);

    mboot_save(mboot_ptr);
    kprint("Mboot saved\n");

    gdt_init();
    kprint("GDT init\n");

    init_vesa();
    kprint("vesa init\n");

    isr_install();
    kprint("ISR init\n");
    
    irq_install();
    kprint("IRQ init\n");

    mem_init();
    kprint("snowflake init\n");

    rtc_init();
    time_gen_boot();
    init_rand();
    kprint("RTC init\n");

    serial_init();
    kprint("serial init\n");

    tasking_init();
    kprint("tasking init\n");

    ramdisk_init();
    kprint("ramdisk init\n");
    
    filesystem_init();
    kprint("filesys init\n");

    init_watfunc();
    kprint("watfunc init\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fskprint("$C~~ version $4%s $C~~\n\n", KERNEL_VERSION);

    // launch of the default program
    run_ifexist(RUN_DEFAULT, 0, NULL);

    task_menu();

    sys_fatal("Nothing to run!");
}
