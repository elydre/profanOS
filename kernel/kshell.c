#include <function.h>
#include <gui/vga.h>
#include <system.h>
#include <string.h>
#include <time.h>
#include <mem.h>

#define BFR_SIZE 66

/* start_kshell() is the last function executed
 * in profanOS if nothing else has worked, there
 * are debug commands but nothing else */

void shell_command(char command[]);

void start_kshell() {
    sys_warning("You are now in the kernel-level shell\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        shell_command(char_buffer);
        char_buffer[0] = '\0';
    }
}

void shell_so(char suffix[]) {
    char path[] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    fskprint("path: %s\n", path);
    sys_run_ifexist(path, 0);    
}

void shell_satan(char suffix[]) {
    int to_rm = 0;
    if (suffix[0] != 's')
        to_rm = ascii_to_int(suffix) * 4;
    int val = mem_get_usable() - mem_get_usage() - to_rm;
    fskprint("memory alloc size: %d ko\n", val);
    mem_alloc(val * 1024);
    mem_print();
}

void shell_help() {
    char *help[] = {
        "GO      - go file as binary",
        "HELP    - show this help",
        "MEM     - show memory state",
        "REBOOT  - reboot the system",
        "SO      - run file in /bin",
        "SATAN   - fill dynamic memory",
    };

    for (int i = 0; i < ARYLEN(help); i++)
        fskprint("%s\n", help[i]);
}

void shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "go") == 0) sys_run_ifexist(suffix, 0);
    else if (str_cmp(prefix, "help") == 0) shell_help();
    else if (str_cmp(prefix, "mem") == 0) mem_print();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0) shell_so(suffix);
    else if (str_cmp(prefix, "satan") == 0) shell_satan(suffix);

    else if (str_cmp(prefix, "") != 0)
        fskprint("$3%s $Bis not a valid command.\n", prefix);
}
