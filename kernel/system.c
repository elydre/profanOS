#include <filesystem.h>
#include <string.h>
#include <system.h>
#include <ports.h>
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
    asm volatile("cli");
    asm volatile("hlt");
}

void do_nothing() {
    asm volatile("sti");
    asm volatile("hlt");
    asm volatile("cli");
}

int sys_warning(char msg[]) {
    fskprint("$DWARNING: $5%s\n", msg);
    return 0;
}

int sys_error(char msg[]) {
    fskprint("$BERROR: $3%s\n", msg);
    return 0;
}

void sys_fatal(char msg[]) {
    fskprint("$CFATAL: $4%s\n", msg);
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
    sys_stop();
}

int sys_run_binary(char path[], int arg, int silence) {
    uint32_t usbl_mem = (uint32_t) mem_get_usable() - mem_get_usage();
    if (usbl_mem < 16 || usbl_mem < fs_get_file_size(path) + 16)
        return sys_error("Not enough memory to run this program");

    char * binary_mem = calloc(fs_get_file_size(path)*126);
    uint32_t * file = fs_declare_read_array(path);

    fs_read_file(path, file);

    for (int i = 0; file[i] != (uint32_t) -1 ; i++)
        binary_mem[i] = (char) file[i];
    
    free(file);

    int old_active_alloc = mem_get_alloc_count() - mem_get_free_count();

    int (*start_program)() = (int (*)())(binary_mem);
    int return_value = start_program(arg);

    if (!silence) {
        if (old_active_alloc < mem_get_alloc_count() - mem_get_free_count())
            sys_warning("Memory leak detected");
        else if (old_active_alloc > mem_get_alloc_count() - mem_get_free_count())
            sys_warning("Memory void detected");
    }
    
    free(binary_mem);
    return return_value;
}

int sys_run_ifexist(char path[], int arg) {
    if (fs_does_path_exists(path) && fs_type_sector(fs_path_to_id(path, 0)) == 2)
        return sys_run_binary(path, arg, 0);
    char ermsg[100];
    str_cpy(ermsg, path);
    str_cat(ermsg, " not found");
    sys_error(ermsg);
    return -1;
}

int sys_get_setting(char name[]) {
    // read settings from /user/settings.txt
    // return -1 if not found
    char * settings = calloc(fs_get_file_size("/user/settings.txt")*126);
    uint32_t * file = fs_declare_read_array("/user/settings.txt");

    fs_read_file("/user/settings.txt", file);

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
    return -1;
}
