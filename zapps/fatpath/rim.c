#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/panda.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>

// input settings
#define SLEEP_T 15
#define FIRST_L 12

// MACROS

#ifndef min
#define min(a, b) ((a) < (b) ? (a) : (b))
#endif

#ifndef max
#define max(a, b) ((a) > (b) ? (a) : (b))
#endif

#define cursor_max_at_line(line) (((line) < g_lines_count - 1) ? \
                                  g_data_lines[(line) + 1] - g_data_lines[line] - 1 : \
                                  g_data_size - g_data_lines[line] - 1)

#define COLOR_T 0x8F    // title
#define COLOR_L 0x8F    // line number
#define COLOR_D 0x0F    // data
#define COLOR_M 0x70    // more data (>)

// GLOBALS

char *g_data;
int g_data_size;

int *g_data_lines;
int g_lines_count;

int g_cursor_line;
int g_cursor_pos;

char *g_current_screen;

int SCREEN_W;
int SCREEN_H;

// FUNCTIONS

void gui_print(uint32_t x, uint32_t y, char *str, char color) {
    while (*str) {
        panda_set_char(x, y, *str, color);
        x++;
        str++;
    }
}  

void set_title(char *path) {
    char *title = malloc(SCREEN_W + 1);
    memset(title, ' ', SCREEN_W);
    memcpy(title, "RIM", 3);
    memcpy(title + 4, path, min(strlen(path), (uint32_t) SCREEN_W - 4));
    title[SCREEN_W] = '\0';
    gui_print(0, 0, title, COLOR_T);
    free(title);
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
    char *new_screen = calloc((SCREEN_H + 1) * (SCREEN_W + 1), sizeof(char));

    int line = 0;
    int max;
    for (int i = from_line; i < to_line; i++) {
        max = cursor_max_at_line(i);
        for (int j = x_offset; j < min(max, x_offset + SCREEN_W + 1); j++) {
            new_screen[line * SCREEN_W + j - x_offset] = g_data[g_data_lines[i] + j];
        }
        line++;
    }
    return new_screen;
}

void display_data(int from_line, int to_line, int x_offset) {
    if (to_line - from_line > SCREEN_H) {
        printf("error: too many lines to display\n");
        return;
    }

    static int old_lo = 0;

    char *new_screen = calc_new_screen(from_line, to_line, x_offset);

    // display data
    int y = 1;
    int pos, line_offset;
    char line_str[15];
    line_str[0] = ' ';
    line_str[1] = ' ';
    itoa(to_line, line_str + 2, 10);

    line_offset = strlen(line_str) - 2;
    for (int i = 0; i <= to_line - from_line - 1; i++) {
        // line content
        for (int j = 0; j < SCREEN_W - line_offset - 1; j++) {
            pos = i * SCREEN_W + j;
            if (g_current_screen == NULL || new_screen[pos] != g_current_screen[pos] || old_lo != line_offset) {
                if (new_screen[pos] == '\0') {
                    panda_set_char(j + line_offset + 1, y, ' ', COLOR_D);
                } else if (j == SCREEN_W - line_offset - 2) {
                    panda_set_char(j + line_offset + 1, y, '>', COLOR_M);
                } else {
                    panda_set_char(j + line_offset + 1, y, new_screen[pos], COLOR_D);
                }
            }
        }

        if (i < to_line - from_line) {
            // line number
            itoa(from_line + i + 1, line_str + 2, 10);
            gui_print(0, y, line_str + strlen(line_str) - line_offset, COLOR_L);
            panda_set_char(line_offset, y, ' ', COLOR_D);
        }

        y++;
    }

    old_lo = line_offset;

    for (int i = y; i <= SCREEN_H; i++) {
        for (int j = 0; j < line_offset; j++) {
            panda_set_char(j, i, ' ', COLOR_L);
        }
        panda_set_char(line_offset, i, ' ', COLOR_D);
    }

    // cursor
    panda_draw_cursor(g_cursor_pos - x_offset + line_offset + 1, g_cursor_line - from_line + 1);

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
        if (g_cursor_line - 2 < y_offset && g_cursor_line > 1) {
            y_offset = g_cursor_line - 2;
        } else if (g_cursor_line + 3 > y_offset + SCREEN_H) {
            y_offset = g_cursor_line + 3 - SCREEN_H;
        }

        // smart scrolling (x axis)
        if (g_cursor_pos - 2 < x_offset && g_cursor_pos > 1) {
            x_offset = g_cursor_pos - 2;
        } else if (g_cursor_pos + 3 > x_offset + SCREEN_W - 8) {
            x_offset = g_cursor_pos + 3 - SCREEN_W + 8;
        } else if (g_cursor_pos == 0) {
            x_offset = 0;
        }

        // display data
        display_data(y_offset, min(g_lines_count, y_offset + SCREEN_H), x_offset);
    }
}

void clear_screen(void) {
    c_kprint("\e[2J");
    fputs("\e[2J", stdout);
    fflush(stdout);
}

void quit(void) {
    free(g_data);
    free(g_data_lines);
    free(g_current_screen);

    clear_screen();
}

int main(int argc, char **argv) {
    char *file = NULL;
    if (argc > 2) {
        printf("Usage: rim [file]\n");
        return 1;
    }

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);
    if (SCREEN_W * SCREEN_H == 0) {
        printf("rim: panda is required\n");
        return 1;
    }
    SCREEN_H--;
    SCREEN_W--;

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

    g_data = calloc(1024, sizeof(char));
    g_data_size = 1;

    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;

    g_cursor_line = 0;
    g_cursor_pos = 0;

    g_current_screen = NULL;

    clear_screen();

    if (file) {
        set_title(file);
        load_file(file);
    } else {
        set_title("DEMO MODE");
    }

    display_data(0, min(g_lines_count, SCREEN_H), 0);

    main_loop(file);

    if (file) free(file);
    quit();

    return 0;
}
