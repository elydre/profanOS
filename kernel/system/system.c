/*****************************************************************************\
|   === system.c : 2024 ===                                                   |
|                                                                             |
|    Kernel system functions                                       .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

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
 *  CPU interrupt information  *
 *                             *
********************************/

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

/*********************************
 *                              *
 *  F2 key - exit current proc  *
 *                              *
*********************************/

void kernel_exit_current(void) {
    uint32_t pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_list_all(pid_list, PROCESS_MAX);
    uint32_t pid, state;

    // sort by pid
    for (int i = 0; i < pid_list_len; i++) {
        for (int j = i + 1; j < pid_list_len; j++) {
            if (pid_list[i] > pid_list[j]) {
                pid = pid_list[i];
                pid_list[i] = pid_list[j];
                pid_list[j] = pid;
            }
        }
    }

    for (int i = pid_list_len - 1; i >= 0; i--) {
        pid = pid_list[i];
        if (pid < 2) continue;
        state = process_get_state(pid);
        if (state == PROC_STATE_INQ || state == PROC_STATE_SLP) {
            process_kill(pid, 143);
            return;
        }
    }
}

/**********************************
 *                               *
 *  Kernel entry and exit funcs  *
 *                               *
**********************************/

uint8_t IN_KERNEL = 1;

int sys_entry_kernel(int tolerate_error) {
    if (IN_KERNEL) {
        if (tolerate_error)
            return 1;
        sys_fatal("Already in kernel mode");
    }
    process_auto_schedule(0); // lock before entering kernel
    IN_KERNEL = 1;
    return 0;
}

int sys_exit_kernel(int tolerate_error) {
    if (!IN_KERNEL) {
        if (tolerate_error)
            return 1;
        sys_fatal("Already in user mode");
    }
    IN_KERNEL = 0;
    process_auto_schedule(1); // unlock after exiting kernel
    return 0;
}

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
    serial_write(SERIAL_PORT_A, msg, str_len(msg));
    return 0;
}

int sys_set_reporter(int (*reporter)(char *)) {
    if (reporter == NULL) {
        reporter = sys_default_reporter;
    }
    reporter_addr = reporter;
    return 0;
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
    RECURSIVE_COUNT++;

    va_list args;
    va_start(args, msg);

    str_cpy(sys_safe_buffer, "\e[33mWARNING: \e[93m");
    kprintf_va2buf(sys_safe_buffer + 19, msg, args);
    str_cat(sys_safe_buffer, "\e[0m\n");

    va_end(args);
    sys_report(sys_safe_buffer);

    RECURSIVE_COUNT--;
}

void sys_error(char *msg, ...) {
    RECURSIVE_COUNT++;

    int current_pid = process_get_pid();

    va_list args;
    va_start(args, msg);

    str_cpy(sys_safe_buffer, "\e[31mERROR: \e[91m");
    kprintf_va2buf(sys_safe_buffer + 17, msg, args);
    str_cat(sys_safe_buffer, "\e[0m\n");

    va_end(args);
    sys_report(sys_safe_buffer);

    RECURSIVE_COUNT--;

    if (current_pid > 1 && process_kill(current_pid, 143)) {
        sys_fatal("Failed to exit process");
    }
}

void sod_interrupt(uint8_t code, int err_code, char *msg);
void sys_interrupt(uint8_t code, int err_code) {
    kprintf_serial("received interrupt %d from cpu\n", code);

    // page fault issue handler
    if (code == 14) {
        scuba_fault_handler(err_code);
        return;
    }

    if (process_get_pid() == 0) {
        sod_interrupt(code, err_code, code < 19 ? interrupts[code] : "?");
        return;
    }

    sys_error("CPU raised interrupt %d (%s)", code, code < 19 ? interrupts[code] : "?");
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

void sys_nothing_todo(void) {
    kcprint("\nNothing to do, stopping profanOS ", 0x0D);
    kcprint(":", 0x0B);
    kcprint("(\n", 0x0D);

    asm volatile("cli");
    asm volatile("hlt");
}

int sys_power(int action) {
    switch (action) {
        case 0:
            sys_reboot();
            break;
        case 1:
            sys_shutdown();
            break;
        default:
            return 1;
    }
    return 0;
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
