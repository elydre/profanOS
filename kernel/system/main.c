/*****************************************************************************\
|   === main.c : 2024 ===                                                     |
|                                                                             |
|    Kernel main function                                          .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <drivers/keyboard.h>
#include <kernel/butterfly.h>
#include <kernel/multiboot.h>
#include <kernel/snowflake.h>
#include <kernel/scubasuit.h>
#include <drivers/diskiso.h>
#include <kernel/process.h>
#include <drivers/serial.h>
#include <drivers/rtc.h>
#include <kernel/afft.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <cpu/isr.h>
#include <cpu/gdt.h>
#include <minilib.h>
#include <system.h>

multiboot_t *g_mboot;

void kernel_main(void *mboot_ptr) {
    g_mboot = mboot_ptr;

    clear_screen();
    kcprint("booting profanOS...\n", 0x07);

    gdt_init();
    init_vesa();
    kcprint("Multiboot info saved, GDT and VESA initialized\n", 0x07);
    cursor_blink(0);

    status_print(isr_install,   "Installing CPU interrupt handlers");
    status_print(irq_install,   "Enabling interrupts");
    status_print(timer_init,    "Initializing CPU timer (PIT)");
    status_print(init_diskiso,  "Loading initial RAM disk");

    status_print(mem_init,      "Initializing Snowflake physical MM");
    status_print(scuba_init,    "Setting up Scubasuit virtual MM");
    status_print(process_init,  "Starting process manager");
    status_print(filesys_init,  "Loading Butterfly filesystem");

    status_print(tef_init,      "Allocating memory for terminal emulator");

    status_print(afft_init,     "Initializing afft system");
    status_print(sys_init,      "Initializing FPU and error reporting");

    status_print(serial_init,   "Enabling serial ports A and B");
    status_print(keyboard_init, "Setting up PS/2 keyboard");
    status_print(rtc_init,      "Initializing real-time clock");

    status_print(mod_init,      "Loading kernel modules");

    kprintf("Kernel finished booting in %d ms\n", timer_get_ms());

    // launch of the default program
    elf_start(RUN_DEF_PATH, 1, (char *[]){RUN_DEF_NAME, NULL}, NULL);

    sys_nothing_todo();
}
