#include <function.h>
#include <system.h>
#include <string.h>
#include <iolib.h>
#include <task.h>
#include <mem.h>

#include <driver/keyboard.h>


#define BFR_SIZE 65

/* start_kshell() is the last function executed
 * in profanOS if nothing else has worked, there
 * are debug commands but nothing else */

int shell_command(char command[]);

#define KB_KEY_UP 0x48
#define KB_KEY_DOWN 0x50
#define KB_KEY_ENTER 0x1C

void start_kshell() {
    // simple menu to switch between tasks with the keyboard arrows
    int selected_task = 0, nb_alive, key;
    while (1) {
        // refresh tasks
        nb_alive = task_get_alive();
        // print tasks
        for (int i = 0; i < nb_alive; i++) {
            ckprint_at(task_get_name(i), 0, i, (i == selected_task) ? 0xB0 : 0x0B);
        }
        key = kb_get_scancode();
        if (key == KB_KEY_UP) {
            selected_task--;
            if (selected_task < 0) {
                selected_task = nb_alive - 1;
            }
        } else if (key == KB_KEY_DOWN) {
            selected_task++;
            if (selected_task >= nb_alive) {
                selected_task = 0;
            }
        } else if (key == KB_KEY_ENTER) {
            clear_screen();
            yield(task_get_pid(selected_task));
        }
        while (kb_get_scancode() == key);
    }

    /* char char_buffer[BFR_SIZE];
    while (1) {
        rainbow_print("kernel-shell> ");
        input(char_buffer, BFR_SIZE, 9);
        fskprint("\n");
        if (shell_command(char_buffer)) return;
        char_buffer[0] = '\0';
    }*/
}

void shell_so(char suffix[]) {
    char path[100] = "/bin/";
    str_cat(path, suffix);
    str_cat(path, ".bin");
    fskprint("path: %s\n", path);
    run_ifexist(path, 0, NULL);
}

void shell_help() {
    char *help[] = {
        "EXIT   - quit the kshell",
        "GO     - go file as binary",
        "HELP   - show this help",
        "MEM    - show memory state",
        "REBOOT - reboot the system",
        "SO     - run file in /bin",
        "YIELD  - yield to * task",
    };

    for (int i = 0; i < ARYLEN(help); i++)
        fskprint("%s\n", help[i]);
}

int shell_command(char command[]) {
    char prefix[BFR_SIZE], suffix[BFR_SIZE];
    str_cpy(prefix, command);
    str_cpy(suffix, command);
    str_start_split(prefix, ' ');
    str_end_split(suffix, ' ');

    if      (str_cmp(prefix, "clear") == 0)  clear_screen();
    else if (str_cmp(prefix, "exit") == 0)   return 1;
    else if (str_cmp(prefix, "go") == 0)     run_ifexist(suffix, 0, NULL);
    else if (str_cmp(prefix, "help") == 0)   shell_help();
    else if (str_cmp(prefix, "mem") == 0)    mem_print();
    else if (str_cmp(prefix, "reboot") == 0) sys_reboot();
    else if (str_cmp(prefix, "so") == 0)     shell_so(suffix);
    else if (str_cmp(prefix, "yield") == 0)  (str_cmp(suffix, "yield") == 0) ? yield(1) : yield(ascii_to_int(suffix));

    else if (prefix[0] != '\0') fskprint("$Bnot found: $3%s\n", prefix);
    
    return 0;
}
