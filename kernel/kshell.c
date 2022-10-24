#include <driver/keyboard.h>
#include <driver/screen.h>
#include <string.h>
#include <task.h>


#define KB_KEY_UP 0x48
#define KB_KEY_DOWN 0x50
#define KB_KEY_ENTER 0x1C

void menu_print_line(int line, int pid, char name[], int gui, int is_selected);
void menu_print_firstline();
void menu_print_lastline();

void start_kshell() {
    // simple menu to switch between tasks with the keyboard arrows
    int selected_task = 0, nb_alive, key;
    while (1) {
        // refresh tasks
        nb_alive = task_get_alive();
        // print tasks
        menu_print_firstline();
        for (int i = 0; i < nb_alive; i++) {
            menu_print_line(i+6, task_get_pid(i), task_get_name(i), task_is_gui(i), i == selected_task);
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
}

void menu_print_line(int line, int pid, char name[], int gui, int is_selected) {
    char line_str[80], str_pid[10];
    int var = 0;
    for (int i = 0; i < 13; i++) line_str[i] = ' ';
    line_str[10] = 186;
    line_str[13] = '\0';
    ckprint_at(line_str, 0, line, 0x0D);
    for (int i = 0; i < 80; i++) line_str[i] = '\0';
    line_str[0] = ' ';
    int_to_ascii(pid, str_pid);
    str_cat(line_str, str_pid);
    var = str_len(line_str);
    for (int i = 0; i < 5 - var; i++) line_str[var + i] = ' ';
    if (gui == 1) str_cat(line_str, "[G]");
    else if (gui == 2) str_cat(line_str, "[S]");
    else str_cat(line_str, "   ");
    str_cat(line_str, "   ");
    str_cat(line_str, name);
    if (is_selected) ckprint_at(line_str, 13, line, 0x0D);
    else ckprint_at(line_str, 13, line, 0x0F);
    for (int i = 0; i < 8; i++) line_str[i] = ' ';
    line_str[3] = 186;
    line_str[5] = '\0';
    ckprint_at(line_str, 66, line, 0x0D);
}

void menu_print_firstline() {
    char line_str[80];
    for (int i = 0; i < 80; i++) line_str[i] = ' ';
    line_str[10] = 201;
    for (int i = 11; i < 69; i++) line_str[i] = 205;
    line_str[69] = 187;
    line_str[70] = '\0';
    ckprint_at(line_str, 0, 5, 0x0D);
}