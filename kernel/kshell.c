#include <system.h>
#include <string.h>
#include <mem.h>

#define BFR_SIZE 66

/* start_kshell() is the last function executed
 * in profanOS if nothing else has worked, there
 * are debug commands but nothing else */

void shell_command(char command[]);

void start_kshell() {
    sys_warning("you are now in the kernel-level shell.\n");
    char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        shell_command(char_buffer);
        char_buffer[0] = '\0';
    }
}

void shell_help() {
    char *help[] = {
        "ADDR    - show addr of var",
        "GO      - go fill as binary",
        "HELP    - show this help",
        "MEM     - show memory info",
        "REBOOT  - reboot the system",
        "STOP    - stop the system",
    };

    for (int i = 0; i < 5; i++)
        fskprint("%s\n", help[i]);
}

void shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "addr") == 0) fskprint("addr: $1%x\n", &command);
    else if (str_cmp(prefix, "go") == 0) sys_run_ifexist(suffix, 0);
    else if (str_cmp(prefix, "help") == 0) shell_help();
    else if (str_cmp(prefix, "mem") == 0) mem_print();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "stop") == 0) sys_shutdown();

    else if (str_cmp(prefix, "") != 0)
        fskprint("$3%s $Bis not a valid command.\n", prefix);
}
