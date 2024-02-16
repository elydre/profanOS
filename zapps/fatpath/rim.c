#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>
#include <unistd.h>

#include <i_vgui.h>

// SETTINGS

#define SCREEN_W 640
#define SCREEN_H 480

#define FONT_W 8
#define FONT_H 16

#define COLOR_BG 0x111111 // ((c_timer_get_ms() * 0xf) & 0xAAAAAA)
#define COLOR_T1 0xffffff
#define COLOR_T2 0xaaaaaa

#define COLOR_F1 0xffff88
#define COLOR_F2 0xaaaa44

// input settings
#define SLEEP_T 15
#define FIRST_L 12

// MACROS

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#define set_pixel(x, y, color) vgui_set_pixel(g_vgui, x, y, color)
#define render_screen() vgui_render(g_vgui, 0)
#define print(x, y, text, color) vgui_print(g_vgui, x, y, text, color)
#define gputc(x, y, c, color, bg) vgui_putc(g_vgui, x, y, c, color, bg)
#define draw_line(x1, y1, x2, y2, color) vgui_draw_line(g_vgui, x1, y1, x2, y2, color)
#define draw_rect(x, y, width, height, color) vgui_draw_rect(g_vgui, x, y, width, height, color)
#define clear_screen(color) vgui_clear(g_vgui, color)

#define cursor_max_at_line(line) ((line < g_lines_count - 1) ? \
                                  g_data_lines[line + 1] - g_data_lines[line] - 1 : \
                                  g_data_size - g_data_lines[line] - 1)

#define PRINTABLE_LINES (SCREEN_H / FONT_H)
#define PRINTABLE_COLS ((SCREEN_W - (FONT_W * 3 + 5)) / FONT_W)

// GLOBALS

vgui_t *g_vgui;
char *g_title;

char *g_data;
int g_data_size;

int *g_data_lines;
int g_lines_count;

int g_cursor_line;
int g_cursor_pos;

char *g_current_screen;

// FUNCTIONS

void draw_interface(void) {
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
    sid_t file = fu_path_to_sid(ROOT_SID, path);
    int file_size = fu_get_file_size(file);
    int read_size = file_size;

    g_data_size = file_size + 1;
    file_size += 1024 - (file_size % 1024);
    // printf("file size: %d\n", file_size);

    g_data = realloc(g_data, file_size);

    fu_file_read(file, g_data, 0, read_size);

    for (int i = 0; i < g_data_size - 1; i++) {
        if (g_data[i] != '\n') continue;
        g_data[i] = '\0';
        g_data_lines[g_lines_count] = i + 1;
        g_lines_count++;
        if (g_lines_count % 1024) continue;
        g_data_lines = realloc(g_data_lines, (g_lines_count + 1024) * sizeof(int));
    }
}

void save_file(char *path) {
    if (g_data_size - 1 == 0) return;

    char *data_copy = malloc(g_data_size);
    memcpy(data_copy, g_data, g_data_size);

    for (int i = 0; i < g_data_size - 1; i++) {
        if (data_copy[i] == '\0') data_copy[i] = '\n';
    }

    sid_t file = fu_path_to_sid(ROOT_SID, path);
    fu_set_file_size(file, g_data_size - 1);
    fu_file_write(file, data_copy, 0, g_data_size - 1);

    free(data_copy);
}

char *calc_new_screen(int from_line, int to_line, int x_offset) {
    char *new_screen = calloc((PRINTABLE_LINES + 1) * (PRINTABLE_COLS + 1), sizeof(char));

    int line = 0;
    int max;
    for (int i = from_line; i < to_line; i++) {
        max = cursor_max_at_line(i);
        for (int j = x_offset; j < min(max, x_offset + PRINTABLE_COLS); j++) {
            new_screen[line * PRINTABLE_COLS + j - x_offset] = g_data[g_data_lines[i] + j];
        }
        line++;
    }
    return new_screen;
}

void display_data(int from_line, int to_line, int x_offset) {
    if (to_line - from_line > PRINTABLE_LINES) {
        printf("error: too many lines to display\n");
        return;
    }

    // clear line numbers
    draw_rect(0, FONT_H + 1, FONT_W * 3, SCREEN_H - FONT_H - 1, COLOR_BG);

    char *new_screen = calc_new_screen(from_line, to_line, x_offset);

    static int old_cursor_x = 0;
    static int old_cursor_y = 0;

    // remove cursor
    draw_line(old_cursor_x, old_cursor_y, old_cursor_x, old_cursor_y + FONT_H, COLOR_BG);

    // display data
    int y = FONT_H + 2;
    int pos;
    char line_str[10];
    line_str[0] = ' ';
    line_str[1] = ' ';
    for (int i = 0; i <= to_line - from_line; i++) {
        // line content
        for (int j = 0; j < PRINTABLE_COLS; j++) {
            if (g_current_screen == NULL || new_screen[i * PRINTABLE_COLS + j] != g_current_screen[i * PRINTABLE_COLS + j]) {
                gputc(FONT_W * 3 + 5 + j * FONT_W, y, new_screen[i * PRINTABLE_COLS + j], COLOR_T1, COLOR_BG);
            }
        }

        if (i + from_line == g_cursor_line) {
            pos = (g_cursor_pos - x_offset) * FONT_W + FONT_W * 3 + 5;
            draw_line(pos, y, pos, y + FONT_H, COLOR_F1);
            old_cursor_x = pos;
            old_cursor_y = y;
        }

        if (i < to_line - from_line) {
            // line number
            itoa(from_line + i + 1, line_str + 2, 10);
            print(0, y, line_str + strlen(line_str) - 3, COLOR_T2);
        }

        y += FONT_H;
    }
    if (g_current_screen != NULL) free(g_current_screen);
    g_current_screen = new_screen;
}

void realloc_buffer(void) {
    if (g_lines_count % 1024 == 0) {
        g_data_lines = realloc(g_data_lines, (g_lines_count + 1024) * sizeof(int));
    }

    if (g_data_size % 1024 == 0 && g_data_size != 0) {
        g_data = realloc(g_data, (g_data_size + 1024) * sizeof(char));
    }
}

void main_loop(char *path) {
    uint8_t shift_pressed = 0;
    int future_cursor_pos;

    int last_key = 0, key_sgt = 0;
    int key, key_ticks = 0;

    int y_offset = 0;
    int x_offset = 0;

    while (1) {
        // wait for key
        key = c_kb_get_scfh();

        if (key == 224 || key == 0) {   // RESEND or 0
            key = key_sgt;
        } else {
            key_sgt = key;
        }

        if (key != last_key) key_ticks = 0;
        else key_ticks++;
        last_key = key;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            usleep(SLEEP_T * 1000);
            continue;
        }

        // realloc buffer if needed
        realloc_buffer();

        // check if key is enter
        if (key == 28) {
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
            // exit
            return;
        }

        // check if key is ctrl
        else if (key == 29) {
            if (path != NULL) {
                save_file(path);
            }
        }

        // check if key is backspace
        else if (key == 14) {
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
            shift_pressed = 1;
        }

        // check if shift is released
        else if (key == 170 || key == 182) {
            shift_pressed = 0;
        }

        // check if key is tab
        else if (key == 15) {
            int spaces = 4 - (g_cursor_pos % 4);
            // add character to data buffer
            for (int i = g_data_size + spaces - 1; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
                g_data[i] = g_data[i - spaces];

            for (int i = 0; i < spaces; i++)
                g_data[g_data_lines[g_cursor_line] + g_cursor_pos + i] = ' ';

            g_data_size += spaces;

            for (int i = g_cursor_line + 1; i < g_lines_count; i++)
                g_data_lines[i] += spaces;

            g_cursor_pos += spaces;
        }

        // check if key is arrow left
        else if (key == 75) {
            // move cursor
            if (g_cursor_pos > 0) {
                g_cursor_pos--;
            }
        }

        // check if key is arrow right
        else if (key == 77) {
            // move cursor
            if (g_cursor_pos < cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos++;
            }
        }

        // check if key is arrow up
        else if (key == 72) {
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
            // move cursor
            if (g_cursor_line < g_lines_count - 1) {
                g_cursor_line++;
            }
            if (g_cursor_pos > cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos = cursor_max_at_line(g_cursor_line);
            }
        }

        // check if key is printable
        else if (key < 58 && key > 0 && profan_kb_get_char(key, shift_pressed)) {
            // add character to data buffer
            for (int i = g_data_size; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
                g_data[i] = g_data[i - 1];

            g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = profan_kb_get_char(key, shift_pressed);
            g_data_size++;

            for (int i = g_cursor_line + 1; i < g_lines_count; i++)
                g_data_lines[i]++;

            g_cursor_pos++;
        } else {
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

void quit(void) {
    vgui_exit(g_vgui);
    free(g_title);
    free(g_data);
    free(g_data_lines);
    free(g_current_screen);

    c_kprint("\e[2J");
    fputs("\e[2J", stdout);
    fflush(stdout);
}

int main(int argc, char **argv) {
    char *file = NULL;
    if (argc > 2) {
        printf("Usage: rim [file]\n");
        return 1;
    }

    if (argc == 2) {
        char *pwd = getenv("PWD");
        if (!pwd) pwd = "/";

        file = assemble_path(pwd, argv[1]);

        sid_t elm = fu_path_to_sid(ROOT_SID, file);

        if (IS_NULL_SID(elm)) {
            elm = fu_file_create(0, file);
            if (IS_NULL_SID(elm)) {
                printf("\e[91m%s\e[31m failed to create file\e[0m\n", file);
                free(file);
                return 1;
            }
        } else if (!fu_is_file(elm)) {
            printf("\e[91m%s\e[31m file not found\e[0m\n", file);
            free(file);
            return 1;
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

    g_current_screen = NULL;

    clear_screen(COLOR_BG);
    draw_interface();

    if (file) {
        set_title(file);
        load_file(file);
    } else {
        set_title("DEMO MODE");
    }

    display_data(0, min(g_lines_count, PRINTABLE_LINES), 0);
    render_screen();

    main_loop(file);

    if (file) free(file);
    quit();

    return 0;
}
