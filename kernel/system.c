#include <libc/filesystem.h>
#include <driver/serial.h>
#include <cpu/ports.h>
#include <string.h>
#include <system.h>
#include <iolib.h>
#include <mem.h>

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

void sys_stop() {
    fskprint("$4profanOS has been stopped $2:$4(");
    serial_debug("SYSTEM", "profanOS has been stopped");
    asm volatile("cli");
    asm volatile("hlt");
}

int sys_warning(char msg[]) {
    fskprint("$DWARNING: $5%s\n", msg);
    serial_debug("WARNING", msg);
    return 0;
}

int sys_error(char msg[]) {
    fskprint("$BERROR: $3%s\n", msg);
    serial_debug("ERROR", msg);
    return 0;
}

void sys_fatal(char msg[]) {
    fskprint("$CFATAL: $4%s\n", msg);
    serial_debug("FATAL", msg);
    sys_stop();
}

void sys_interrupt(int code) {
    /* do not use this function, is
    * reserved for cpu interrupts*/

    char msg[30];
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

    if (code < 19) str_cpy(msg, interrupts[code]);
    else str_cpy(msg, "Reserved");
    fskprint("$CCPU INTERRUPT $4%d$C: $4%s\n", code, msg);
    serial_debug("CPU INTERRUPT", msg);
    sys_stop();
}

int sys_get_setting(char name[]) {
    // read settings from /sys/settings.txt
    // return 0 if not found
    char * settings = calloc(fs_get_file_size("/sys/settings.txt")*126);
    uint32_t * file = fs_declare_read_array("/sys/settings.txt");

    fs_read_file("/sys/settings.txt", file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        settings[i] = (char) file[i];

    free(file);

    char line[100];
    char arg[100];
    int line_i = 0;
    int part = 0;
    for (int i = 0; i < str_len(settings); i++) {
        if (part == 0) {
            if (settings[i] == '=') {
                part = 1;
                line[line_i] = '\0';
                line_i = 0;
            } else {
                line[line_i] = settings[i];
                line_i++;
            }
            continue;
        }
        if (settings[i] == '\n') {
            part = 0;
            arg[line_i] = '\0';
            line_i = 0;
            if (str_cmp(line, name))
                continue;
            free(settings);
            if (arg[str_len(arg)-1] == '\r')
                arg[str_len(arg)-1] = '\0';
            return ascii_to_int(arg);
        } else {
            arg[line_i] = settings[i];
            line_i++;
        }
    }
    free(settings);
    sys_warning("Setting not found");
    return 0;
}
