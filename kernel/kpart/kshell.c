#include <kernel/snowflake.h>
#include <kernel/scubasuit.h>
#include <drivers/diskiso.h>
#include <drivers/serial.h>
#include <kernel/process.h>
#include <cpu/timer.h>
#include <gui/gnrtx.h>
#include <cpu/timer.h>
#include <gui/vesa.h>
#include <minilib.h>
#include <system.h>

#define BFR_SIZE 65

void kernel_switch_back(void) {
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;

    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (process_get_state(pid) < 3 && pid && pid != process_get_pid()) {
            process_sleep(pid, 0);
        }
    }

    if (process_get_state(0) > 2) {
        // if the kernel is sleeping
        kprint("\n");
        process_handover(0);
    } else if (process_get_pid() > 2) {
        kprint("\n");
        process_sleep(process_get_pid(), 0);
    }
}

void kernel_exit_current(void) {
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;

    for (int i = pid_list_len - 1; i >= 0; i--) {
        pid = pid_list[i];
        if (process_get_state(pid) == PROCESS_RUNNING && pid) {
            force_exit_pid(pid, 130);
            return;
        }
    }
    for (int i = pid_list_len - 1; i >= 0; i--) {
        pid = pid_list[i];
        if (process_get_state(pid) == PROCESS_TSLPING && pid) {
            force_exit_pid(pid, 130);
            return;
        }
    }
}

void shell_so(char *suffix) {
    char path[100] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    kprintf("path: %s\n", path);
    run_ifexist(path, 0, (char **)0);
}

void shell_help(void) {
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

void shell_addr(void) {
    kprintf("vesa fb: %x\n", vesa_get_framebuffer());
    kprintf("max add: %x (%dMo)\n", mem_get_info(0, 0), mem_get_info(0, 0) / 1024 / 1024);
    kprintf("diskiso: %x (%dMo)\n", diskiso_get_start(), diskiso_get_size() / 1024 / 1024);
    kprintf("mm base: %x\n", MEM_BASE_ADDR);
    kprintf("watdily: %x\n", WATDILY_ADDR);
    kprintf("watfunc: %x\n", WATFUNC_ADDR);
}

void shell_mem(void) {
    allocated_part_t *mem_parts = (void *) mem_get_info(3, 0);
    int index = 0;
    while (mem_parts[index].state) {
        kprintf("part %d (s: %d, t: %d) -> %x, size: %x\n",
            index,
            mem_parts[index].state,
            mem_parts[index].task_id,
            mem_parts[index].addr,
            mem_parts[index].size
        );
        index = mem_parts[index].next;
    }
}

int shell_command(char *command) {
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

    else if (str_cmp(prefix, "h") == 0) process_handover(str2int(suffix));
    else if (str_cmp(prefix, "k") == 0) process_kill(str2int(suffix));
    else if (str_cmp(prefix, "w") == 0) process_wakeup(str2int(suffix));

    else if (prefix[0] != '\0') kprintf("not found: %s\n", prefix);

    return 0;
}

void start_kshell(void) {
    sys_warning("You are now in the kernel-level shell");
    kprint("\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        krainbow("kernel-shell> ");
        kinput(char_buffer, BFR_SIZE);
        kprint("\n");
        if (shell_command(char_buffer)) break;
        char_buffer[0] = '\0';
    }
    kprint("exiting kshell can cause a kernel panic\n");
}
