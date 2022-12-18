#include <libc/ramdisk.h>
#include <gui/gnrtx.h>
#include <gui/vesa.h>
#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
#include <mem.h>

#define BFR_SIZE 65

int shell_command(char command[]);

void start_kshell() {
    sys_warning("You are now in the kernel-level shell");
    fskprint("\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        if (shell_command(char_buffer)) break;
        char_buffer[0] = '\0';
    }
    clear_screen();
}


void shell_so(char suffix[]) {
    char path[100] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    fskprint("path: %s\n", path);
    run_ifexist(path, 0, (char **)0);    
}

void shell_help() {
    char *help[] = {
        "ADDR   - show main address",
        "ALLOC  - allocate *0x1000",
        "EXIT   - quit the kshell",
        "GO     - go file as binary",
        "HELP   - show this help",
        "MEM    - show memory state",
        "REBOOT - reboot the system",
        "SO     - run file in /bin",
    };

    for (int i = 0; i < ARYLEN(help); i++)
        fskprint("%s\n", help[i]);
}

void shell_addr() {
    fskprint("vesa fb: %x\n", vesa_get_framebuffer());
    fskprint("max add: %x (%fMo)\n", mem_get_info(0, 0), mem_get_info(0, 0) / 1024.0 / 1024.0);
    fskprint("ramdisk: %x (%fMo)\n", ramdisk_get_address(), ramdisk_get_size() / 2048.0);
    fskprint("mm base: %x\n", MEM_BASE_ADDR);
    fskprint("watfunc: %x\n", WATFUNC_ADDR);
    fskprint("rand sv: %x\n", RAND_SAVE);
}

int shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "addr") == 0) shell_addr();
    else if (str_cmp(prefix, "alloc") == 0) malloc(ascii_to_int(suffix) * 0x1000);
    else if (str_cmp(prefix, "exit") == 0) return 1;
    else if (str_cmp(prefix, "go") == 0) run_ifexist(suffix, 0, (char **)0);
    else if (str_cmp(prefix, "help") == 0) shell_help();
    else if (str_cmp(prefix, "mem") == 0) mem_print();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);
    else if (prefix[0] != '\0') fskprint("$Bnot found: $3%s\n", prefix);

    return 0;
}

