#include <kernel/filesystem.h>
#include <kernel/multiboot.h>
#include <kernel/snowflake.h>
#include <kernel/ramdisk.h>
#include <driver/serial.h>
#include <kernel/task.h>
#include <driver/rtc.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <cpu/gdt.h>
#include <system.h>

#include <iolib.h>
#include <type.h>

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
    kprint("RTC init\n");

    serial_init();
    kprint("serial init\n");

    tasking_init();
    kprint("tasking init\n");

    ramdisk_init();
    kprint("ramdisk init\n");
    
    filesystem_init();
    kprint("filesys init\n");

    dily_load("/lib/iolib.bin", 1000);
    dily_load("/lib/string.bin", 1001);
    dily_load("/lib/setting.bin", 1002);
    dily_load("/lib/mem.bin", 1003);
    dily_load("/lib/time.bin", 1004);
    kprint("zlibs init\n");

    init_watfunc();
    kprint("watfunc init\n");

    rainbow_print("\n\nWelcome to profanOS!\n");
    fsprint("$C~~ version $4%s $C~~\n\n", KERNEL_VERSION);

    // launch of the default program
    run_ifexist(RUN_DEFAULT, 0, NULL);

    task_menu();

    sys_fatal("Nothing to run!");
}
