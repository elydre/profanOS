#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>

#include <i_vgui.h>
#include <i_time.h>

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define set_pixel(x, y, color) vgui_set_pixel(g_vgui, x, y, color)
#define render_screen() vgui_render(g_vgui, 1)
#define print(x, y, text, color) vgui_print(g_vgui, x, y, text, color)
#define draw_line(x1, y1, x2, y2, color) vgui_draw_line(g_vgui, x1, y1, x2, y2, color)
#define draw_rect(x, y, width, height, color) vgui_draw_rect(g_vgui, x, y, width, height, color)
#define clear_screen(color) vgui_clear(g_vgui, color)

#define cursor_max_at_line(line) ((line < g_lines_count - 1) ? \
                                  g_data_lines[line + 1] - g_data_lines[line] - 1 : \
                                  g_data_size - g_data_lines[line] - 1)

#define SCREEN_W 320
#define SCREEN_H 200

#define FONT_W 8
#define FONT_H 16

#define PRINTABLE_LINES (SCREEN_H / FONT_H)
#define PRINTABLE_COLS ((SCREEN_W - (FONT_W * 3 + 5)) / FONT_W)

#define COLOR_BG 0x111111
#define COLOR_T1 0xffffff
#define COLOR_T2 0xaaaaaa

#define COLOR_F1 0xffff88
#define COLOR_F2 0xaaaa44

vgui_t *g_vgui;
char *g_title;

char *g_data;
int g_data_size;

int *g_data_lines;
int g_lines_count;

int g_cursor_line;
int g_cursor_pos;

void draw_interface() {
    draw_line(0, FONT_H, SCREEN_W - 1, FONT_H, COLOR_F2);
    draw_line(FONT_W * 3 + 2, FONT_H, FONT_W * 3 + 2, SCREEN_H, COLOR_F2);
}

void set_title(char *path) {
    strcpy(g_title, "rim - ");
    strcat(g_title, path);

    draw_rect(0, 0, SCREEN_H, FONT_H, COLOR_BG);
    print(0, 0, g_title, COLOR_T1);
}

void load_file(char *path) {
    int file_size = c_fs_get_file_size(path);
    g_data_size = file_size + 1;
    file_size += 1024 - (file_size % 1024);
    printf("file size: %d\n", file_size);

    g_data = realloc(g_data, file_size);

    c_fs_read_file(path, (uint8_t *) g_data);

    for (int i = 0; i < file_size; i++) {
        if (g_data[i] != '\n') continue;
        g_data[i] = '\0';
        g_data_lines[g_lines_count] = i + 1;
        g_lines_count++;
        if (g_lines_count % 1024) continue;
        g_data_lines = realloc(g_data_lines, g_lines_count + 1024);
    }
}

void save_file(char *path) {
    char *data_copy = malloc(g_data_size);
    memcpy(data_copy, g_data, g_data_size);

    for (int i = 0; i < g_data_size - 1; i++) {
        if (data_copy[i] == '\0') data_copy[i] = '\n';
    }

    c_fs_write_in_file(path, (uint8_t *) data_copy, g_data_size - 1);
    free(data_copy);
}

void display_data(int from_line, int to_line, int x_offset) {
    // clear screen
    draw_rect(FONT_W * 3 + 5, FONT_H + 1, SCREEN_W - FONT_W * 3, SCREEN_H - FONT_H - 1, COLOR_BG);
    draw_rect(0, FONT_H + 1, FONT_W * 3, SCREEN_H - FONT_H - 1, COLOR_BG);

    // display data
    int y = FONT_H + 2;
    int pos;
    char line_str[10];
    line_str[0] = ' ';
    line_str[1] = ' ';
    for (int i = from_line; i < to_line; i++) {
        // line content
        if (x_offset <= cursor_max_at_line(i)) {
            print(FONT_W * 3 + 5, y, g_data + g_data_lines[i] + x_offset, COLOR_T1);
            if (i == g_cursor_line) {
                pos = (g_cursor_pos - x_offset) * FONT_W + FONT_W * 3 + 5;
                draw_line(pos, y, pos, y + FONT_H, COLOR_F1);
            }
        }
        // line number
        itoa(i + 1, line_str + 2, 10);
        print(0, y, line_str + strlen(line_str) - 3, COLOR_T2);

        y += FONT_H;
    }
}

void realloc_buffer() {
    if (g_lines_count % 1024 == 0) {
        printf("reallocating data lines with size %d\n", g_lines_count + 1024);
        g_data_lines = realloc(g_data_lines, (g_lines_count + 1024) * sizeof(int));
    }

    if (g_data_size % 1024 == 0 && g_data_size != 0) {
        printf("reallocating data with size %d\n", g_data_size + 1024);
        g_data = realloc(g_data, (g_data_size + 1024) * sizeof(char));
    }
}

void main_loop(char *path) {
    int key, future_cursor_pos;
    uint8_t shift_pressed = 0;
    char c;

    int y_offset = 0;
    int x_offset = 0;

    while (1) {
        // wait for key
        key = c_kb_get_scfh();

        // realloc buffer if needed
        realloc_buffer();
    
        // check if key is enter
        if (key == 28) {
            c_serial_print(SERIAL_PORT_A, "enter pressed\n");
            // add line to data lines

            // add '\0' character to data buffer
            for (int i = g_data_size; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
                g_data[i] = g_data[i - 1];

            g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = '\0';
            g_data_size++;

            // add line to data lines
            for (int i = g_lines_count; i > g_cursor_line + 1; i--) {
                g_data_lines[i] = g_data_lines[i - 1];
                g_data_lines[i]++;
            }

            g_data_lines[g_cursor_line + 1] = g_data_lines[g_cursor_line] + g_cursor_pos + 1;
            g_lines_count++;
            g_cursor_line++;
            g_cursor_pos = 0;
        }

        // check if key is escape
        else if (key == 1) {
            c_serial_print(SERIAL_PORT_A, "escape pressed\n");
            // exit
            return;
        }

        // check if key is ctrl
        else if (key == 29) {
            c_serial_print(SERIAL_PORT_A, "ctrl pressed\n");
            if (path != NULL) {
                save_file(path);
            }
        }

        // check if key is backspace
        else if (key == 14) {
            c_serial_print(SERIAL_PORT_A, "backspace pressed\n");
            // remove character from data buffer
            if (g_cursor_pos > 0) {
                for (int i = g_data_lines[g_cursor_line] + g_cursor_pos; i < g_data_size; i++)
                    g_data[i - 1] = g_data[i];
                
                for (int i = g_cursor_line + 1; i < g_lines_count; i++)
                    g_data_lines[i]--;

                g_data_size--;
                g_cursor_pos--;
            } else if (g_cursor_line > 0) {
                future_cursor_pos = cursor_max_at_line(g_cursor_line - 1);

                for (int i = g_data_lines[g_cursor_line]; i < g_data_size; i++)
                    g_data[i - 1] = g_data[i];
                
                // remove line from data lines
                for (int i = g_cursor_line; i < g_lines_count - 1; i++) {
                    g_data_lines[i] = g_data_lines[i + 1];
                    g_data_lines[i]--;
                }

                g_data_size--;
                g_lines_count--;
                g_cursor_line--;
                g_cursor_pos = future_cursor_pos;
            }
        }

        // check if shift is pressed
        else if (key == 42 || key == 54) {
            c_serial_print(SERIAL_PORT_A, "shift pressed\n");
            shift_pressed = 1;
        }

        // check if shift is released
        else if (key == 170 || key == 182) {
            c_serial_print(SERIAL_PORT_A, "shift released\n");
            shift_pressed = 0;
        }

        // check if key is tab
        else if (key == 15) {
            c_serial_print(SERIAL_PORT_A, "tab pressed\n");
            int spaces = 4 - (g_cursor_pos % 4);
            for (int i = 0; i < spaces; i++) {
                // add character to data buffer
                for (int i = g_data_size; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
                    g_data[i] = g_data[i - 1];

                g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = ' ';
                g_data_size++;

                g_cursor_pos++;
            }
        }

        // check if key is arrow left
        else if (key == 75) {
            c_serial_print(SERIAL_PORT_A, "arrow left pressed\n");
            // move cursor
            if (g_cursor_pos > 0) {
                g_cursor_pos--;
            }
        }

        // check if key is arrow right
        else if (key == 77) {
            c_serial_print(SERIAL_PORT_A, "arrow right pressed\n");
            // move cursor
            if (g_cursor_pos < cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos++;
            }
        }

        // check if key is arrow up
        else if (key == 72) {
            c_serial_print(SERIAL_PORT_A, "arrow up pressed\n");
            // move cursor
            if (g_cursor_line > 0) {
                g_cursor_line--;
            }
            if (g_cursor_pos > cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos = cursor_max_at_line(g_cursor_line);
            }
        }

        // check if key is arrow down
        else if (key == 80) {
            c_serial_print(SERIAL_PORT_A, "arrow down pressed\n");
            // move cursor
            if (g_cursor_line < g_lines_count - 1) {
                g_cursor_line++;
            }
            if (g_cursor_pos > cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos = cursor_max_at_line(g_cursor_line);
            }
        }

        // check if key is printable
        else if (key < 58 && key > 0) {
            c = c_kb_scancode_to_char(key, shift_pressed);

            // add character to data buffer
            for (int i = g_data_size; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
                g_data[i] = g_data[i - 1];

            g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = c;
            g_data_size++;

            for (int i = g_cursor_line + 1; i < g_lines_count; i++)
                g_data_lines[i]++;

            g_cursor_pos++;
        } else {
            ms_sleep(20);
            continue;
        }

        // smart scrolling (y axis)
        if (g_cursor_line - 1 < y_offset && g_cursor_line > 0) {
            y_offset = g_cursor_line - 1;
        } else if (g_cursor_line + 3 > y_offset + PRINTABLE_LINES) {
            y_offset = g_cursor_line + 3 - PRINTABLE_LINES;
        }

        // smart scrolling (x axis)
        if (g_cursor_pos - 1 < x_offset && g_cursor_pos > 0) {
            x_offset = g_cursor_pos - 1;
        } else if (g_cursor_pos + 3 > x_offset + PRINTABLE_COLS) {
            x_offset = g_cursor_pos + 3 - PRINTABLE_COLS;
        } else if (g_cursor_pos == 0) {
            x_offset = 0;
        }
      
        // display data
        display_data(y_offset, min(g_lines_count, y_offset + PRINTABLE_LINES), x_offset);

        // render screen
        render_screen();
    }
}

void quit() {
    vgui_exit(g_vgui);
    free(g_title);
    free(g_data);
    free(g_data_lines);
}

int main(int argc, char *argv[]) {
    char *file = NULL;
    if (argc == 3) {
        file = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
        assemble_path(argv[1], argv[2], file);

        if (!(c_fs_does_path_exists(file) && c_fs_get_sector_type(c_fs_path_to_id(file)) == 2)) {
            printf("$3%s$B file not found\n", file);
            free(file);
            return 0;
        }
    }

    vgui_t vgui = vgui_setup(SCREEN_W, SCREEN_H);
    g_vgui = &vgui;

    g_title = calloc(256, sizeof(char));

    g_data = calloc(1024, sizeof(char));
    g_data_size = 1;

    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;

    g_cursor_line = 0;
    g_cursor_pos = 0;

    clear_screen(COLOR_BG);
    draw_interface();

    if (file) {
        set_title(file);
        load_file(file);
    } else {
        set_title("DEMO MODE");
    }

    display_data(0, g_lines_count, 0);
    render_screen();

    main_loop(file);

    if (file) free(file);
    quit();

    return 0;
}
