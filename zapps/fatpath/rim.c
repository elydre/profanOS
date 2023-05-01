#include <syscall.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#include <i_vgui.h>
#include <i_time.h>


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

#define COLOR_BG 0x111111
#define COLOR_FG 0xffffff
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
}

void set_title(char *title) {
    int old_len = strlen(g_title);
    draw_rect(0, 0, old_len * FONT_W, FONT_H, COLOR_BG);
    print(0, 0, title, COLOR_FG);
    strcpy(g_title, title);
}

void display_data(int from_line, int to_line) {
    // clear screen
    draw_rect(0, FONT_H + 1, SCREEN_W, SCREEN_H - FONT_H - 1, COLOR_BG);

    // display data
    int y = FONT_H + 1;
    int pos;
    for (int i = from_line; i < to_line; i++) {
        print(0, y, g_data + g_data_lines[i], COLOR_FG);
        if (i == g_cursor_line) {
            pos = g_cursor_pos * FONT_W;
            draw_line(pos, y, pos, y + FONT_H, COLOR_F1);
        }
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

void main_loop() {
    int key, future_cursor_pos;
    uint8_t shift_pressed = 0;
    char c;
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
      
        // display data
        display_data(0, g_lines_count);

        // render screen
        render_screen();
    }
}

int main(int argc, char *argv[]) {
    vgui_t vgui = vgui_setup(SCREEN_W, SCREEN_H);
    g_vgui = &vgui;

    g_title = calloc(256, sizeof(char));

    g_data = calloc(1024, sizeof(char));
    g_data_size = 1;

    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;

    clear_screen(COLOR_BG);
    draw_interface();

    set_title("rim - (nothing open)");

    render_screen();
    main_loop();

    vgui_exit(&vgui);
    free(g_title);

    return 0;
}
