#include <syscall.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <i_vgui.h>
#include <stdio.h>
#include <math.h>

int str_ncmp(char s1[], char s2[], int n) {
    // TODO: recode this
    int i;
    for (i = 0; s1[i] == s2[i]; i++) {
        if (i == n || s1[i] == '\0') return 0;
    }
    if (i == n) return 0;
    return s1[i] - s2[i];
}

#define ENTER 28
#define BACKSPACE 14

#define TEXT_COL (1024/8-1)
#define TEXT_LINE (768/16 - 1)

int global_argc;
char **global_argv;
char **text_buffer;

void draw_mode(vgui_t *vgui, char *value) {
    if (strlen(value) > 12) {
        return;
    }
    vgui_draw_rect(vgui, 0, 768-16, 8*12, 16, 0x000000);
    vgui_print(vgui, 0, 768-16, value, 0xFFFFFF);
}

int handle_command(vgui_t *vgui, char *command) {
    // for now we only handle the exit command
    if (strcmp(command, ":exit") == 0) {
        return 1; // exit
    }
    if (!str_ncmp(command, ":save", 5)) {
        while (!(*command == ' ')) command++;
        command++;
        char *new_path = calloc(100, sizeof(char));
        strcpy(new_path, global_argv[1]);
        strcat(new_path, command);
        c_serial_print(SERIAL_PORT_A, new_path);
        FILE *file = fopen(new_path, "w");
        char *into_file = calloc(TEXT_COL*TEXT_LINE*2, sizeof(char));
        int into_file_index = 0;
        for (int i = 0; i < TEXT_LINE; i++) {
            for (int j = 0; j < TEXT_COL; j++) {
                if (text_buffer[i][j] == '\0') {
                    break;
                }
                into_file[into_file_index] = text_buffer[i][j];
                into_file_index++;
            }
            if (text_buffer[i+1][0] == '\0') {
                break;
            }
            into_file[into_file_index] = '\n';
            into_file_index++;
        }
        into_file[into_file_index] = '\0';
        fwrite(into_file, sizeof(char), strlen(into_file), file);
        fclose(file);
        free(new_path);
        return 1; // on close après
    }
    return 0;
}

void main(int argc, char **argv) {
    global_argc = argc;
    global_argv = argv;
    // preparation
    argv += 2;
    argc -= 2;

    // initial draw
    c_clear_screen();
    vgui_t vgui = vgui_setup(1024, 768);
    vgui_draw_line(&vgui, 0, 768-17, 1024, 768-17, 0x00FF00);
    vgui_draw_line(&vgui, 8*12, 768-17, 8*12, 768, 0x00FF00);
    vgui_render(&vgui, 0);

    // mainloop
    int scancode;
    bool command_mode = 2;
    bool command_type = 0;

    // text buffer
    char *command_buffer = calloc(100, sizeof(char));
    text_buffer = calloc(TEXT_LINE, sizeof(char *));
    for (int i = 0; i < TEXT_LINE; i++) {
        text_buffer[i] = calloc(TEXT_COL, sizeof(char));
    }
    int cursor_line = 0;
    int cursor_column = 0;

    while ((scancode = c_kb_get_scfh()) != 1) {
        if (scancode == 0) {
            continue;
        }
        // control flow things
        else if (scancode == KB_CTRL || command_mode == 2) {
            command_mode = !command_mode;
            if (command_mode) {
                draw_mode(&vgui, "MODE COMMAND");
            } else {
                draw_mode(&vgui, "MODE TYPING");
            }
            vgui_render(&vgui, 0);
        }
        else if (command_mode && scancode == 52) { // :
            // we add the char to command_buffer
            command_type = 1;
            command_buffer[0] = ':';
            command_buffer[1] = '\0';
            vgui_print(&vgui, 8*12, 768-16, command_buffer, 0xFFFFFF);
            vgui_render(&vgui, 0);
        }

        else if (command_type && scancode == ENTER) {
            // we void command buffer
            command_type = 0;
            
            int result = handle_command(&vgui, command_buffer);
            if (result == 1) {
                break;
            }

            command_buffer[0] = '\0';
            vgui_draw_rect(&vgui, 8*12+1, 768-16, 1024, 16, 0x000000);
            vgui_render(&vgui, 0);
        }

        else if (command_type && scancode == BACKSPACE ) {
            if (strlen(command_buffer) > 0) {
                command_buffer[strlen(command_buffer) - 1] = '\0';
                vgui_draw_rect(&vgui, 8*12+1, 768-16, 1024, 16, 0x000000);
                vgui_print(&vgui, 8*12, 768-16, command_buffer, 0xFFFFFF);
                vgui_render(&vgui, 0);
            }
        }

        // on traite les caractères comme il faut
        else if (command_type && scancode <= SC_MAX) {
            if (strlen(command_buffer) > 100) {
                continue;
            }
            command_buffer[strlen(command_buffer) + 1] = '\0';
            command_buffer[strlen(command_buffer)] = c_kb_scancode_to_char(scancode, 0);
            vgui_print(&vgui, 8*12, 768-16, command_buffer, 0xFFFFFF);
            vgui_render(&vgui, 0);
        }

        else if (!command_mode && scancode == ENTER) {
            // we move the cursor a colomn down if we can
            if (cursor_line < TEXT_LINE) {
                cursor_line++;
                cursor_column = 0;
            }
        }

        else if (!command_mode && scancode <= SC_MAX) {
            if (text_buffer[cursor_line][cursor_column] != '\0') {
                continue;
            }
            if (c_kb_scancode_to_char(scancode, 0) == '?') {
                continue;
            }
            text_buffer[cursor_line][cursor_column] = c_kb_scancode_to_char(scancode, 0);
            // we clean the char
            vgui_draw_rect(&vgui, 8*cursor_column, 16*cursor_line, 8, 16, 0x000000);
            // we print the char
            vgui_print(&vgui, 8*cursor_column, 16*cursor_line, &text_buffer[cursor_line][cursor_column], 0xFFFFFF);
            vgui_render(&vgui, 0);
            // we move the cursor iif it's not at the end
            if (cursor_column < TEXT_COL) {
                cursor_column++;
            }
        }
        
    }

    // end
    vgui_exit(&vgui);
    free(command_buffer);
    for (int i = 0; i < TEXT_LINE; i++) {
        free(text_buffer[i]);
    }
    free(text_buffer);
    c_clear_screen();
}
