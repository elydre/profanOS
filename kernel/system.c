#include <kernel/scubasuit.h>
#include <driver/serial.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <cpu/ports.h>
#include <gui/gnrtx.h>
#include <minilib.h>

#include <system.h>

/****************************
 * panic and reboot system *
****************************/

void sys_stop() {
    serial_kprintf("FATAL during process %d\n", process_get_pid());

    ckprint("profanOS has been stopped ", 0x0D);
    ckprint(":", 0x0B);
    ckprint("(\n", 0x0D);

    serial_debug("SYSTEM", "profanOS has been stopped");
    asm volatile("cli");
    asm volatile("hlt");
}

int sys_warning(char *msg) {
    ckprint("WARNING: ", 0x06);
    ckprint(msg, 0x0E);
    kprint("\n");

    serial_debug("WARNING", msg);
    return 0;
}

int sys_error(char *msg) {
    ckprint("ERROR: ", 0x04);
    ckprint(msg, 0x0C);
    kprint("\n");

    serial_debug("ERROR", msg);
    return 0;
}

void sys_fatal(char *msg) {
    ckprint("FATAL: ", 0x05);
    ckprint(msg, 0x0D);
    kprint("\n");

    serial_debug("FATAL", msg);
    sys_stop();
}

void sys_interrupt(int code, int err_code) {
    /* do not use this function, is
    * reserved for cpu interrupts*/

    serial_kprintf("received interrupt %d from cpu\n", code);

    // page fault issue handler
    if (code == 14) {
        scuba_fault_handler(err_code);
        return;
    }

    ckprint("CPU INTERRUPT ", 0x05);

    char msg[30];
    int2str(code, msg);
    ckprint(msg, 0x0D);
    ckprint(": ", 0x05);

    char *interrupts[] = {
        "Division by zero",
        "Debug",
        "Non-maskable interrupt",
        "Breakpoint",
        "Overflow",
        "Out of bounds",
        "Invalid opcode",
        "No coprocessor",
        "Double fault",
        "Coprocessor segment overrun",
        "Bad TSS",
        "Segment not present",
        "Stack fault",
        "General protection fault",
        "Page fault",
        "Unknown interrupt",
        "Coprocessor fault",
        "Alignment check",
        "Machine check",
    };

    if (code < 19) ckprint(interrupts[code], 0x0D);
    else ckprint("Reserved", 0x0D);
    kprint("\n");

    sys_stop();
}

void sys_reboot() {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    asm volatile("hlt");
}

void sys_shutdown() {
    port_word_out(0x604, 0x2000);   // qemu
    port_word_out(0xB004, 0x2000);  // bochs
    port_word_out(0x4004, 0x3400);  // virtualbox
    sys_stop();                     // halt if above didn't work
}

void cpuid(uint32_t eax, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    asm volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(eax));
}

int sys_init_fpu() {
    // get if fpu is present
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);
    if (!(edx & (1 << 24))) return 1;

    // enable fpu
    asm volatile("fninit");
    asm volatile("fwait");
    asm volatile("clts");
    asm volatile("mov %cr0, %eax");
    asm volatile("and $0x9FFFFFFF, %eax");
    asm volatile("mov %eax, %cr0");
    asm volatile("mov %cr4, %eax");
    asm volatile("or $0x600, %eax");
    asm volatile("mov %eax, %cr4");

    return 0;
}

void sys_kinfo(char *dest) {
    str_cpy(dest, KERNEL_EDITING " " KERNEL_VERSION);
}
