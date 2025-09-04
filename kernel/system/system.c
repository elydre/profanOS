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
#include <kernel/multiboot.h>
#include <kernel/butterfly.h>
#include <kernel/process.h>
#include <kernel/tinyelf.h>
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

uint32_t g_in_kernel_total = 0;
uint32_t g_last_entry = 0;

void sys_entry_kernel(void) {
    asm volatile("cli");        // disable all interrupts

    if (IN_KERNEL)
        sys_fatal("Already in kernel mode");

    IN_KERNEL = 1;

    g_last_entry = TIMER_TICKS;

    port_write8(0x21, 0xFE);    // allow only IRQ0
    port_write8(0xA1, 0xFF);    // disable all IRQ8–15

    asm volatile("sti");        // re-enable interrupts, but only IRQ0 will be handled
}

void sys_exit_kernel(int restore_pic) {
    asm volatile("cli");        // disable all interrupts

    if (!IN_KERNEL)
        sys_fatal("Already in user mode");

    schedule_if_needed();

    IN_KERNEL = 0;

    g_in_kernel_total += TIMER_TICKS - g_last_entry;

    port_write8(0x21, 0x00);    // allow all IRQs
    port_write8(0xA1, 0x00);    // allow all IRQs

    if (restore_pic) {
        if (restore_pic > 40)
            port_write8(0xA0, 0x20); // slave
        port_write8(0x20, 0x20);     // master
    }

    asm volatile("sti");        // re-enable interrupts
}

uint32_t sys_get_kernel_time(void) {
    return g_in_kernel_total + (TIMER_TICKS - g_last_entry);
}

/********************************
 *                             *
 *  error reporting functions  *
 *                             *
********************************/

char sys_safe_buffer[256];

int sys_default_reporter(char *msg) {
    kprint(msg);
    return 0;
}

void *reporter_addr = sys_default_reporter;
int RECURSIVE_COUNT = 0;

int sys_set_reporter(int (*reporter)(char *)) {
    if (reporter == NULL)
        reporter_addr = sys_default_reporter;
    else
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
        sys_warning("Invalid reporter, using the default one");
    }

    if (((int (*)(char *)) reporter_addr)(copy)) {
        RECURSIVE_COUNT++;
        sys_report(copy);
        RECURSIVE_COUNT--;
    }

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
        good = port_read8(0x64);
    port_write8(0x64, 0xFE);
    asm volatile("hlt");
}

void sys_shutdown(void) {
    port_write16(0x604, 0x2000);   // qemu
    port_write16(0xB004, 0x2000);  // bochs
    port_write16(0x4004, 0x3400);  // virtualbox

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

static void cpuid(uint32_t eax, uint32_t *a, uint32_t *b, uint32_t *c, uint32_t *d) {
    asm volatile("cpuid" : "=a"(*a), "=b"(*b), "=c"(*c), "=d"(*d) : "a"(eax));
}

static int init_fpu(void) {
    // get if fpu is present
    uint32_t eax, ebx, ecx, edx;
    cpuid(1, &eax, &ebx, &ecx, &edx);

    if (!(edx & (1 << 24)))
        return 1;

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

static int write_kernel_version(void) {
    if (IS_SID_NULL(kfu_dir_create(0, "/sys", "kernel")))
        return 1;

    uint32_t sid = kfu_file_create("/sys/kernel", "version.txt");

    if (IS_SID_NULL(sid))
        return 1;

    char *version = KERNEL_VERSION "\n" KERNEL_EDITING "\ni386\n";
    uint32_t version_len = str_len(version);

    if (fs_cnt_set_size(sid, version_len))
        return 1;

    if (fs_cnt_write(sid, version, 0, version_len))
        return 1;

    return 0;
}

int sys_init(void) {
    init_fpu();
    write_kernel_version();
    return 0;
}

/********************************
 *                             *
 *  multiboot elf resolution   *
 *                             *
********************************/

uint32_t sys_name2addr(const char *name) {
    if (!(g_mboot->flags & (1 << 5)))
        return 0; // no elf section info

    Elf32_Shdr *sh = (Elf32_Shdr *)g_mboot->elf_sec.addr;

    for (uint32_t i = 0; i < g_mboot->elf_sec.num; i++) {
        if (sh[i].sh_type != SHT_SYMTAB)
            continue;

        Elf32_Sym *sym = (Elf32_Sym *) sh[i].sh_addr;
        char *strtab = (char *) sh[sh[i].sh_link].sh_addr;

        int symcount = sh[i].sh_size / sh[i].sh_entsize;

        for (int j = 0; j < symcount; j++) {
            if (sym[j].st_name == 0)
                continue;
            if (str_cmp(name, strtab + sym[j].st_name) == 0)
                return sym[j].st_value;
        }
    }

    return 0;
}

const char *sys_addr2name(uint32_t addr) {
    if (!(g_mboot->flags & (1 << 5)))
        return NULL; // no elf section info

    Elf32_Shdr *sh = (Elf32_Shdr *)g_mboot->elf_sec.addr;

    for (uint32_t i = 0; i < g_mboot->elf_sec.num; i++) {
        if (sh[i].sh_type != SHT_SYMTAB)
            continue;

        Elf32_Sym *sym = (Elf32_Sym *)sh[i].sh_addr;
        char *strtab = (char *) sh[sh[i].sh_link].sh_addr;

        int symcount = sh[i].sh_size / sh[i].sh_entsize;

        for (int j = 0; j < symcount; j++) {
            if (addr < sym[j].st_value || addr >= sym[j].st_value + sym[j].st_size)
                continue;
            if (sym[j].st_name == 0)
                return NULL;
            return strtab + sym[j].st_name;
        }
    }

    return NULL;
}
