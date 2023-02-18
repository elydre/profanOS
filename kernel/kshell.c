#include <kernel/snowflake.h>
#include <kernel/ramdisk.h>
#include <kernel/process.h>
#include <driver/diskiso.h>
#include <driver/serial.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <cpu/timer.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

#include <i_iolib.h>

#define BFR_SIZE 65

void kernel_switch_back() {
    /*int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;

    kprint("\n");

    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (process_get_state(pid) < 2 && pid) {
            process_sleep(pid);
            sprintf("Process %d stopped\n", pid);
        }
    }*/
}

int shell_command(char command[]);

void start_kshell() {
    sys_warning("You are now in the kernel-level shell");
    kprint("\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        kprint("\n");
        if (shell_command(char_buffer)) break;
        char_buffer[0] = '\0';
    }
    kprint("exiting kshell can cause a kernel panic\n");
}


void shell_so(char suffix[]) {
    char path[100] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    kprintf("path: %s\n", path);
    run_ifexist(path, 0, (char **)0);    
}

void shell_help() {
    char *help[] = {
        "ADDR   - show main address",
        "ALLOC  - allocate *0x1000",
        "EXIT   - quit the kshell",
        "GO     - go file as binary",
        "HELP   - show this help",
        "MEM    - show memory allocs",
        "REBOOT - reboot the system",
        "SO     - run file in /bin",
    };

    for (int i = 0; i < ARYLEN(help); i++) {
        kprintf("%s\n", help[i]);
    }
}

void shell_addr() {
    kprintf("vesa fb: 0x%x\n", vesa_get_framebuffer());
    kprintf("max add: 0x%x (%dMo)\n", mem_get_info(0, 0), mem_get_info(0, 0) / 1024 / 1024);
    kprintf("ramdisk: 0x%x (%dMo)\n", ramdisk_get_address(), ramdisk_get_size() / 2048);
    kprintf("diskiso: 0x%x (%dMo)\n", diskiso_get_start(), diskiso_get_size() / 2048);
    kprintf("mm base: 0x%x\n", MEM_BASE_ADDR);
    kprintf("watdily: 0x%x\n", WATDILY_ADDR);
    kprintf("watfunc: 0x%x\n", WATFUNC_ADDR);
}

void shell_mem() {
    allocated_part_t *mem_parts = (void *) mem_get_info(3, 0);
    int index = 0;
    while (mem_parts[index].state) {
        kprintf("part %d (s: %d, t: %d) -> 0x%x, size: %d\n",
            index,
            mem_parts[index].state,
            mem_parts[index].task_id,
            mem_parts[index].addr,
            mem_parts[index].size
        );
        index = mem_parts[index].next;
    }
}

void process_func() {
    while (1) {
        sprintf("%d ms passed\n", timer_get_ms());
        process_sleep(1, 100);
    }
}

void test_process() {
    int pid = process_create(process_func, 1, "test_process");
    process_wakeup(pid);
}


int shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    int part = 0;
    int i;

    for (i = 0; i < str_len(command); i++) {
        if (command[i] == ' ') {
            prefix[i] = '\0';
            part = 1;
        }
        else if (part == 0) prefix[i] = command[i];
        else if (part == 1) suffix[i - str_len(prefix) - 1] = command[i];
    }
    if (part == 0) prefix[i] = '\0';
    else suffix[i - str_len(prefix) - 1] = '\0';

    if      (str_cmp(prefix, "addr") == 0) shell_addr();
    else if (str_cmp(prefix, "alloc") == 0) malloc(str2int(suffix) * 0x1000);
    else if (str_cmp(prefix, "exit") == 0) return 1;
    else if (str_cmp(prefix, "go") == 0) run_ifexist(suffix, 0, (char **)0);
    else if (str_cmp(prefix, "help") == 0) shell_help();
    else if (str_cmp(prefix, "mem") == 0) shell_mem();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);
    else if (str_cmp(prefix, "proc") == 0) test_process();
    else if (str_cmp(prefix, "sleep") == 0) process_sleep(1, 1000);

    else if (prefix[0] != '\0') kprintf("not found: %s\n", prefix);

    return 0;
}
