#include <drivers/keyboard.h>
#include <kernel/butterfly.h>
#include <kernel/multiboot.h>
#include <kernel/snowflake.h>
#include <kernel/scubasuit.h>
#include <drivers/diskiso.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <drivers/mouse.h>
#include <drivers/rtc.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <cpu/gdt.h>
#include <minilib.h>
#include <system.h>


void kernel_main(void *mboot_ptr) {
    clear_screen();
    kcprint("booting profanOS...\n", 0x07);

    mboot_save(mboot_ptr);
    gdt_init();
    init_vesa();
    kcprint("Multiboot info saved, GDT and VESA initialized\n", 0x07);
    cursor_blink(0);

    status_print(isr_install,  "Installing", "cpu interrupts");
    status_print(irq_install,  "Enabling", "interruptions");
    status_print(timer_init,   "Initing", "cpu timer (PIT)");
    status_print(keyboard_init,"Setuping", "PS/2 keyboard");
    status_print(mouse_init,   "Setuping", "PS/2 mouse");
    status_print(serial_init,  "Enabling", "serial port (A and B)");
    status_print(init_diskiso, "Loading", "initial ramdisk");
    status_print(mem_init,     "Initing", "snowflake physical MM");
    status_print(scuba_init,   "Setuping", "scubasuit virtual MM");
    status_print(tef_init,     "Allocing mem", "for terminal emulator");
    status_print(rtc_init,     "Initing", "real time clock");
    status_print(sys_init,     "Initing", "FPU and error reporting");
    status_print(process_init, "Starting", "process manager");
    status_print(filesys_init, "Loading", "butterfly filesystem");
    status_print(init_watfunc, "Initing", "watfunc");

    kprintf("successfully booted in %d ms\n", timer_get_ms());

    // launch of the default program
    run_ifexist(RUN_DEFAULT, 1, NULL, NULL);

    start_kshell();

    sys_fatal("Nothing to run!");
}
