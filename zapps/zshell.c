#include "addf.h"

#define BFR_SIZE 68

int shell_command(int addr, char command[]);

int start(int addr, int arg) {
    char char_buffer[BFR_SIZE];
    INIT_AF(addr);
    AF_rainbow_print();
    AF_fskprint();
    AF_input();

    while (1) {
        rainbow_print("zshell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        if (shell_command(addr, char_buffer))
            break;
        char_buffer[0] = '\0';
    }
    return arg;
}

void shell_help(int addr) {
    INIT_AF(addr);
    AF_fskprint();

    char *help[] = {
        "CLEAR   - clear the screen",
        "ECHO    - print the arguments",
        "EXIT    - quit the zshell",
        "HELP    - show this help",
    };

    for (int i = 0; i < 4; i++)
        fskprint("$4%s\n", help[i]);
}

int shell_command(int addr, char command[]) {
    INIT_AF(addr);

    AF_str_start_split();
    AF_str_end_split();
    AF_clear_screen();
    AF_fskprint();
    AF_str_cpy();
    AF_str_cmp();

    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if (str_cmp(prefix, "exit") == 0) return 1;
    else if (str_cmp(prefix, "echo") == 0) fskprint("$4%s\n", suffix);
    else if (str_cmp(prefix, "clear") == 0) clear_screen();
    else if (str_cmp(prefix, "help") == 0) shell_help(addr);

    else if (str_cmp(prefix, "") != 0)
        fskprint("$3%s $Bis not a valid command.\n", prefix);

    return 0;
}
