#include <driver/keyboard.h>
#include <driver/screen.h>
#include <libc/task.h>
#include <string.h>
#include <time.h>

#define KB_KEY_UP 0x48
#define KB_KEY_DOWN 0x50
#define KB_KEY_ENTER 0x1C

void menu_print_line(int line, int task_i, int is_selected);
void menu_print_decoline(int type, int line);

void task_menu() {
    // simple menu to switch between tasks with the keyboard arrows
    int selected_task = 0, nb_alive, key;
    while (1) {
        // refresh tasks
        nb_alive = task_get_alive();
        // print tasks
        cursor_blink(1);
        menu_print_decoline(0, 5);
        for (int i = 0; i < nb_alive; i++) menu_print_line(i+6, i, i == selected_task);
        for (int i = nb_alive; i < 13; i++) menu_print_decoline(1, i+6);
        menu_print_decoline(2, 19);
        key = kb_get_scancode();
        if (key == KB_KEY_UP) selected_task--;
        else if (key == KB_KEY_DOWN) selected_task++;
        if (selected_task >= nb_alive) selected_task = 0;
        if (selected_task < 0) selected_task = nb_alive - 1;
        if (key == KB_KEY_ENTER) {
            clear_screen();
            cursor_blink(0);
            if (selected_task == 0) start_kshell();
            else task_switch(task_get_pid(selected_task));
        }
        while (kb_get_scancode() == KB_KEY_UP || kb_get_scancode() == KB_KEY_DOWN || kb_get_scancode() == KB_KEY_ENTER);
    }
}

void menu_print_line(int line, int task_i, int is_selected) {
    int gui = task_is_gui(task_i);
    char line_str[80], str_pid[10];
    int var = 0;
    for (int i = 0; i < 13; i++) line_str[i] = ' ';
    line_str[10] = 186;
    line_str[13] = '\0';
    ckprint_at(line_str, 0, line, 0x0D);
    for (int i = 0; i < 80; i++) line_str[i] = '\0';
    line_str[0] = ' ';
    int_to_ascii(task_get_pid(task_i), str_pid);
    str_cat(line_str, str_pid);
    var = str_len(line_str);
    for (int i = 0; i < 5 - var; i++) line_str[var + i] = ' ';
    if (gui == 1) str_cat(line_str, "[G]");
    else if (gui == 2) str_cat(line_str, "[S]");
    else str_cat(line_str, "   ");
    str_cat(line_str, "   ");
    str_cat(line_str, task_get_name(task_i));
    if (is_selected) ckprint_at(line_str, 13, line, 0x0D);
    else ckprint_at(line_str, 13, line, 0x0F);
    for (int i = 0; i < 8; i++) line_str[i] = ' ';
    line_str[3] = 186;
    line_str[5] = '\0';
    ckprint_at(line_str, 66, line, 0x0D);
}

void menu_print_decoline(int type, int line) {
    /* type 0 = first line
     * type 1 = center line
     * type 2 = last line */
    char line_str[80];
    for (int i = 0; i < 80; i++) line_str[i] = ' ';
    line_str[10] = (type == 0) ? 201 : (type == 1) ? 186 : 200;
    for (int i = 11; i < 69; i++) line_str[i] = (type == 1) ? ' ' : 205;
    line_str[69] = (type == 0) ? 187 : (type == 1) ? 186 : 188;
    line_str[70] = '\0';
    ckprint_at(line_str, 0, line, 0x0D);
}
