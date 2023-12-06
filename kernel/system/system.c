#include <kernel/scubasuit.h>
#include <drivers/serial.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <cpu/ports.h>
#include <gui/gnrtx.h>
#include <minilib.h>
#include <system.h>

/********************************
 *                             *
 *  error reporting functions  *
 *                             *
********************************/

char sys_safe_buffer[256];
void *reporter_addr;

int RECURSIVE_COUNT;

int sys_default_reporter(char *msg) {
    kprint(msg);
    serial_print(SERIAL_PORT_A, msg);
    return 0;
}

void sys_set_reporter(int (*reporter)(char *)) {
    if (reporter == NULL) {
        reporter = sys_default_reporter;
    }
    reporter_addr = reporter;
}

void sys_report(char *msg) {
    char *copy = malloc(str_len(msg) + 1);
    str_cpy(copy, msg);

    if (reporter_addr != sys_default_reporter &&
        (reporter_addr == NULL ||
        ((uint8_t *) reporter_addr)[0] != 0x55 ||
        ((uint8_t *) reporter_addr)[1] != 0x89 ||
        RECURSIVE_COUNT > 1)
    ) {
        reporter_addr = sys_default_reporter;
        sys_warning("Invalid reporter address, using default reporter");
    }

    ((int (*)(char *)) reporter_addr)(copy);
    free(copy);
}

void sys_warning(char *msg, ...) {
    process_disable_sheduler();
    RECURSIVE_COUNT++;

    va_list args;
    va_start(args, msg);

    str_cpy(sys_safe_buffer, "\033[33mWARNING: \033[93m");
    kprintf_va2buf(sys_safe_buffer + 19, msg, args);
    str_cat(sys_safe_buffer, "\033[0m\n");

    va_end(args);
    sys_report(sys_safe_buffer);

    RECURSIVE_COUNT--;
    process_enable_sheduler();
}

void sys_error(char *msg, ...) {
    process_disable_sheduler();
    RECURSIVE_COUNT++;

    int current_pid = process_get_pid();

    va_list args;
    va_start(args, msg);

    str_cpy(sys_safe_buffer, "\033[31mERROR: \033[91m");
    kprintf_va2buf(sys_safe_buffer + 17, msg, args);
    str_cat(sys_safe_buffer, "\033[0m\n");

    va_end(args);
    sys_report(sys_safe_buffer);

    RECURSIVE_COUNT--;
    process_enable_sheduler();

    if (current_pid > 1 && force_exit_pid(current_pid, 130)) {
        sys_fatal("Failed to exit process");
    }
}

void sod_interrupt(int code, int err_code);
void sys_interrupt(int code, int err_code) {
    kprintf_serial("received interrupt %d from cpu\n", code);

    // page fault issue handler
    if (code == 14) {
        scuba_fault_handler(err_code);
        return;
    }

    sod_interrupt(code, err_code);
}

/********************************
 *                             *
 *  stop and reboot functions  *
 *                             *
********************************/

void sys_reboot(void) {
    uint8_t good = 0x02;
    while (good & 0x02)
        good = port_byte_in(0x64);
    port_byte_out(0x64, 0xFE);
    asm volatile("hlt");
}

void sys_shutdown(void) {
    port_word_out(0x604, 0x2000);   // qemu
    port_word_out(0xB004, 0x2000);  // bochs
    port_word_out(0x4004, 0x3400);  // virtualbox

    kcprint("profanOS has been stopped ", 0x0D);
    kcprint(":", 0x0B);
    kcprint("(\n", 0x0D);

    asm volatile("cli");
    asm volatile("hlt");
}

/******************************
 *                           *
 *  FPU and CPUID functions  *
 *                           *
******************************/

void cpuid(uint32_t eax, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    asm volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(eax));
}

int sys_init(void) {
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

    RECURSIVE_COUNT = 0;
    sys_set_reporter(sys_default_reporter);

    return 0;
}

/********************************
 *                             *
 *  information get functions  *
 *                             *
********************************/

char *sys_kinfo(void) {
    return KERNEL_EDITING " " KERNEL_VERSION;
}
