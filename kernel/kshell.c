#include <kernel/snowflake.h>
#include <kernel/ramdisk.h>
#include <kernel/process.h>
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
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;

    kprint("\n");

    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (process_get_state(pid) < 2 && pid) {
            process_sleep(pid);
            sprintf("Process %d stopped\n", pid);
        }
    }
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
    clear_screen();
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
        "PK     - kill a process",
        "PS     - show process list",
        "PW     - wake up a process",
        "REBOOT - reboot the system",
        "SO     - run file in /bin",
    };

    for (int i = 0; i < ARYLEN(help); i++)
        kprintf("%s\n", help[i]);
}

void shell_addr() {
    fsprint("vesa fb: %x\n", vesa_get_framebuffer());
    fsprint("max add: %x (%fMo)\n", mem_get_info(0, 0), mem_get_info(0, 0) / 1024.0 / 1024.0);
    fsprint("ramdisk: %x (%fMo)\n", ramdisk_get_address(), ramdisk_get_size() / 2048.0);
    fsprint("mm base: %x\n", MEM_BASE_ADDR);
    fsprint("watfunc: %x\n", WATFUNC_ADDR);
}

void shell_process_show() {
    char *state[] = {
        "RUNNING",
        "WAITING",
        "SLEEPING",
        "ZOMBIE",
    };
    int pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = process_generate_pid_list(pid_list, PROCESS_MAX);
    int pid;
    char name[64];
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        process_get_name(pid, name);
        kprintf("pid: %d, ppid: %d, state: %s, name: %s\n", pid, process_get_ppid(pid), state[process_get_state(pid)], name);
    }
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
    else if (str_cmp(prefix, "pk") == 0) process_kill(str2int(suffix));
    else if (str_cmp(prefix, "ps") == 0) shell_process_show();
    else if (str_cmp(prefix, "pw") == 0) process_wakeup(str2int(suffix));
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);

    else if (prefix[0] != '\0') kprintf("not found: %s\n", prefix);

    return 0;
}
