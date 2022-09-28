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

void sys_warning(int code, char msg[]) {
    fskprint("$DWARNING $5%d$D: $5%s\n", code, msg);
}

void sys_error(int code, char msg[]) {
    fskprint("$BERROR $3%d$B: $3%s\n", code, msg);
}

void sys_fatal(int code, char msg[]) {
    fskprint("$CFATAL $4%d$C: $4%s\n", code, msg);
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

int sys_run_binary(char *fileName, int arg) {
	char * binary_mem = calloc(fs_get_file_size(fileName)*126);
	uint32_t * file = fs_declare_read_array(fileName);

	fs_read_file(fileName, file);

	for (int i = 0; file[i] != (uint32_t) -1 ; i++)
		binary_mem[i] = (char) file[i];

    int old_active_alloc = mem_get_alloc_count() - mem_get_free_count();

	int (*start_program)() = (int (*)())(binary_mem);
	int return_value = start_program((int) wf_get_func_addr, arg);

    if (old_active_alloc != mem_get_alloc_count() - mem_get_free_count())
        sys_warning(42, "Memory leak detected");
    
    free(binary_mem);
    free(file);

    return return_value;
}
