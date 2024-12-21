/*****************************************************************************\
|   === rim.c : 2024 ===                                                      |
|                                                                             |
|    A simple CLI text editor with syntax highlighting             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/panda.h>
#include <profan/clip.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdio.h>
#include <ctype.h>

// input settings
#define SLEEP_T 20
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
                                  g_data_count - g_data_lines[line] - 1)

#define COLOR_T 0x70    // title
#define COLOR_L 0x87    // line number
#define COLOR_D 0x0F    // data
#define COLOR_M 0x70    // more data (>)
#define COLOR_U 0x80    // unknown character
#define COLOR_W 0x08    // whitespace

#define RIM_VERSION "7 rev 2"

// GLOBALS
typedef struct {
    uint8_t words;
    uint8_t numbers;
    uint8_t funccalls;
    uint8_t braces;
    uint8_t strings;
    uint8_t ctypes;
    char **keywords;
    char **blues;
} rim_syntax_t;

typedef struct {
    rim_syntax_t *syntax;
    int always_tab;
    int save_at_exit;
} rim_settings_t;

rim_settings_t g_rim;

char *g_data;
int g_data_count;
int g_data_max;

int *g_data_lines;
int g_lines_count;
int g_lines_max;

int g_cursor_line;
int g_cursor_pos;

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

void init_data(void) {
    free(g_data);
    g_data = malloc(1024);
    g_data_count = 1;
    g_data_max = 1024;

    free(g_data_lines);
    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;
    g_lines_max = 1024;
}

void load_file(char *path) {
    int read_size, fd;

    init_data();

    if (path) {
        fd = open(path, O_RDONLY | O_CREAT);
        if (fd < 0)
            exit(1);
    } else {
        fd = 0;
    }

    while ((read_size = read(fd, g_data + g_data_count - 1, 1024))) {
        if (read_size < 0) {
            close(fd);
            exit(1);
        }

        g_data_count += read_size;
        if (g_data_count >= g_data_max) {
            g_data_max += 1024;
            g_data = realloc(g_data, g_data_max);
        }
    }

    close(fd);

    for (int i = 0; i < g_data_count - 1; i++) {
        if (g_data[i] != '\n')
            continue;
        g_data[i] = '\0';
        g_data_lines[g_lines_count] = i + 1;
        g_lines_count++;
        if (g_lines_count >= g_lines_max) {
            g_lines_max += 1024;
            g_data_lines = realloc(g_data_lines, g_lines_max * sizeof(int));
        }
    }
}

void save_file(char *path) {
    int fd;

    if (path) {
        fd = open(path, O_WRONLY | O_CREAT | O_TRUNC);
        if (fd < 0)
            return;
    } else {
        fd = 1;
    }

    char *data_copy = malloc(g_data_count);
    memcpy(data_copy, g_data, g_data_count);

    for (int i = 0; i < g_data_count - 1; i++) {
        if (data_copy[i] == '\0') data_copy[i] = '\n';
    }

    write(fd, data_copy, g_data_count - 1);
    close(fd);

    free(data_copy);
}

int word_isnumber(char *word, int size) {
    if (!g_rim.syntax->numbers) return 0;
    if (size > 2 && word[0] == '0' && (word[1] == 'x' || word[1] == 'X')) {
        for (int i = 2; i < size; i++) {
            if (!isxdigit(word[i])) return 0;
        }
        return 1;
    }
    for (int i = 0; i < size; i++) {
        if (!isdigit(word[i])) return 0;
    }
    return 1;
}

int word_purple(char *word, uint32_t size) {
    if (!g_rim.syntax->keywords) return 0;
    for (uint32_t i = 0; g_rim.syntax->keywords[i]; i++) {
        if (size == strlen(g_rim.syntax->keywords[i]) && !memcmp(word, g_rim.syntax->keywords[i], size)) return 1;
    }
    return 0;
}

int word_isblue(char *word, uint32_t size) {
    if (g_rim.syntax->ctypes && size > 2 && word[size - 1] == 't' && word[size - 2] == '_') return 1;
    if (!g_rim.syntax->blues) return 0;
    for (uint32_t i = 0; g_rim.syntax->blues[i]; i++) {
        if (size == strlen(g_rim.syntax->blues[i]) && !memcmp(word, g_rim.syntax->blues[i], size)) return 1;
    }
    return 0;
}

int word_isbrace(char *word, uint32_t size) {
    if (!g_rim.syntax->braces) return 0;
    return (size == 1 && (
        word[0] == '(' ||
        word[0] == ')' ||
        word[0] == '{' ||
        word[0] == '}' ||
        word[0] == '[' ||
        word[0] == ']'
    ));
}

int word_paraft(char *word, uint32_t size) {
    if (!g_rim.syntax->funccalls) return 0;
    for (uint32_t i = size; word[i]; i++) {
        if (word[i] == '(') return 1;
        if (!isspace(word[i])) return 0;
    }
    return 0;
}

void put_word(int line, int in_word, uint16_t *new_screen, int new_screen_i, char *word, int size) {
    if (size == 0) return;
    char color = 0x0F;
    if (in_word == 2 && g_rim.syntax->strings) color = 0x0E;
    else if (in_word == 1 && g_rim.syntax->words) {
        color = 0x07;
        if (word_isnumber(word, size)) color = 0x0A;
        else if (word_isblue(word, size)) color = 0x09;
        else if (word_purple(word, size)) color = 0x0D;
        else if (word_paraft(word, size)) color = 0x06;
    } else if (word_isbrace(word, size)) color = 0x0B;

    if (line > new_screen_i) {
        size = size - (line - new_screen_i);
        if (size <= 0) return;
        word += line - new_screen_i;
        new_screen_i = line;
    }

    for (int k = 0; k < size; k++) {
        new_screen[new_screen_i + k] = word[k] | (color << 8);
    }
}

#define localput_word(X) put_word((i - from_line) * SCREEN_W, in_word, new_screen, \
    line * SCREEN_W + x + world_start - x_offset, g_data + g_data_lines[i] + world_start, X)

uint16_t *calc_new_screen(int from_line, int to_line, int x_offset, int *cursor_patch) {
    uint16_t *new_screen = calloc((SCREEN_H + 1) * (SCREEN_W + 1), sizeof(uint16_t));

    int line = 0;
    int max, x;

    int world_start, in_word;
    char chr_str;

    for (int i = from_line; i < to_line; i++) {
        max = min(cursor_max_at_line(i), x_offset + SCREEN_W + 1);
        x = 0;
        in_word = world_start = 0;
        for (int j = 0; j < x_offset; j++) {
            if (in_word == 2 && g_data[g_data_lines[i] + j] != chr_str) {
                in_word = 0;
            }
            else if (g_data[g_data_lines[i] + j] == '\"' || g_data[g_data_lines[i] + j] == '\'') {
                chr_str = g_data[g_data_lines[i] + j];
                in_word = 2;
            }
            if (in_word == 2)
                continue;
            if (isalnum(g_data[g_data_lines[i] + j]) || g_data[g_data_lines[i] + j] == '_') {
                if (!in_word) {
                    in_word = 1;
                    world_start = j;
                }
            } else {
                in_word = 0;
            }
        }

        for (int j = x_offset; j < max; j++) {
            if (g_data[g_data_lines[i] + j] == '\t') {
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2) in_word = 0;
                for (int k = 0; k < 4; k++) {
                    new_screen[line * SCREEN_W + j - x_offset + x] = '>' | (COLOR_W << 8);
                    if ((j + x) % 4 == 3) break;
                    x++;
                    if (i == g_cursor_line && j + x <= g_cursor_pos + *cursor_patch)
                        (*cursor_patch)++;
                }
                continue;
            }
            if (!isprint(g_data[g_data_lines[i] + j])) {
                new_screen[line * SCREEN_W + j - x_offset + x] = '?' | (COLOR_U << 8);
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2) in_word = 0;
                continue;
            }

            else if (g_data[g_data_lines[i] + j] == ' ') {
                new_screen[line * SCREEN_W + j - x_offset + x] = '.' | (COLOR_W << 8);
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2) in_word = 0;
                continue;
            }

            if (in_word == 2) {
                if (g_data[g_data_lines[i] + j] == chr_str) {
                    localput_word(j - world_start + 1);
                    world_start = j + 1;
                    in_word = 0;
                }
                continue;
            }

            else if (g_data[g_data_lines[i] + j] == '\"' || g_data[g_data_lines[i] + j] == '\'') {
                localput_word(j - world_start);
                chr_str = g_data[g_data_lines[i] + j];
                world_start = j;
                in_word = 2;
            }

            else if (in_word != 2) {
                if (isalnum(g_data[g_data_lines[i] + j]) || g_data[g_data_lines[i] + j] == '_') {
                    if (!in_word) {
                        localput_word(j - world_start);
                        world_start = j;
                        in_word = 1;
                    }
                } else {
                    localput_word(j - world_start);
                    world_start = j;
                    in_word = 0;
                }
            }
        }
        localput_word(max - world_start);
        line++;
    }
    return new_screen;
}

void display_data(int from_line, int to_line, int x_offset) {
    if (to_line - from_line > SCREEN_H) {
        fputs("rim: error: too many lines to display\n", stderr);
        exit(1);
    }

    int cursor_patch = 0;

    uint16_t *new_screen = calc_new_screen(from_line, to_line, x_offset, &cursor_patch);

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
            if (new_screen[pos] == 0) {
                panda_set_char(j + line_offset + 1, y, ' ', COLOR_D);
            } else if (j == SCREEN_W - line_offset - 2) {
                panda_set_char(j + line_offset + 1, y, '>', COLOR_M);
            } else {
                panda_set_char(j + line_offset + 1, y, new_screen[pos] & 0xFF, new_screen[pos] >> 8);
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

    for (int i = y; i <= SCREEN_H; i++) {
        for (int j = line_offset; j < SCREEN_W; j++) {
            panda_set_char(j, i, ' ', COLOR_D);
        }
        for (int j = 0; j < line_offset; j++) {
            panda_set_char(j, i, ' ', COLOR_L);
        }
        panda_set_char(line_offset, i, ' ', COLOR_D);
    }

    // cursor
    panda_draw_cursor(g_cursor_pos - x_offset + line_offset + 1 + cursor_patch, g_cursor_line - from_line + 1);

    free(new_screen);
}

void realloc_buffer(void) {
    if (g_lines_count >= g_lines_max - 1) {
        if (g_lines_count > g_lines_max) {
            fputs("rim: g_data_lines, overflow detected\n", stderr);
            exit(1);
        }
        g_data_lines = realloc(g_data_lines, (g_lines_max + 1024) * sizeof(int));
        g_lines_max += 1024;
    }

    if (g_data_count >= g_data_max - 1) {
        if (g_data_count > g_data_max) {
            fputs("rim: g_data, overflow detected\n", stderr);
            exit(1);
        }
        g_data = realloc(g_data, g_data_max + 1024);
        g_data_max += 1024;
    }
}

void insert_tab(void) {
    int tab = 0;

    if (g_rim.always_tab) {
        tab = 1;
    } else for (int j = g_data_lines[g_cursor_line]; g_data[j] != '\0'; j++) {
        if (g_data[j] == '\t') {
            tab = 1;
            break;
        }
    }

    if (tab) {
        // add character to data buffer
        for (int i = g_data_count; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
            g_data[i] = g_data[i - 1];

        g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = '\t';
        g_data_count++;

        for (int i = g_cursor_line + 1; i < g_lines_count; i++)
            g_data_lines[i]++;

        g_cursor_pos++;
    } else {
        int spaces = 4 - (g_cursor_pos % 4);
        // realloc buffer if needed
        if (g_data_count + spaces >= g_data_max) {
            g_data_max += 1024;
            g_data = realloc(g_data, g_data_max * sizeof(char));
        }

        // add character to data buffer
        for (int i = g_data_count + spaces - 1; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
            g_data[i] = g_data[i - spaces];

        for (int i = 0; i < spaces; i++)
            g_data[g_data_lines[g_cursor_line] + g_cursor_pos + i] = ' ';

        g_data_count += spaces;

        for (int i = g_cursor_line + 1; i < g_lines_count; i++)
            g_data_lines[i] += spaces;

        g_cursor_pos += spaces;
    }
}

void insert_newline(void) {
    // add '\0' character to data buffer
    for (int i = g_data_count; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
        g_data[i] = g_data[i - 1];

    g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = '\0';
    g_data_count++;

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

void insert_char(char chr) {
    // add character to data buffer
    for (int i = g_data_count; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
        g_data[i] = g_data[i - 1];

    g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = chr;
    g_data_count++;

    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i]++;

    g_cursor_pos++;
}

void execute_backspace(void) {
    int future_cursor_pos;

    // remove character from data buffer
    if (g_cursor_pos > 0) {
        for (int i = g_data_lines[g_cursor_line] + g_cursor_pos; i < g_data_count; i++)
            g_data[i - 1] = g_data[i];

        for (int i = g_cursor_line + 1; i < g_lines_count; i++)
            g_data_lines[i]--;

        g_data_count--;
        g_cursor_pos--;
    } else if (g_cursor_line > 0) {
        future_cursor_pos = cursor_max_at_line(g_cursor_line - 1);

        for (int i = g_data_lines[g_cursor_line]; i < g_data_count; i++)
            g_data[i - 1] = g_data[i];

        // remove line from data lines
        for (int i = g_cursor_line; i < g_lines_count - 1; i++) {
            g_data_lines[i] = g_data_lines[i + 1];
            g_data_lines[i]--;
        }

        g_data_count--;
        g_lines_count--;
        g_cursor_line--;
        g_cursor_pos = future_cursor_pos;
    }
}

int execute_ctrl(int key, uint8_t shift, char *path) {
    int start, end, size;

    char c = profan_kb_get_char(key, shift);

    if (c == 'q') { // quit
        return 1;
    }

    if (c == 's') { // save
        if (path)
            save_file(path);
    }

    if (c == 'c' || c == 'x') { // copy or cut
        start = g_data_lines[g_cursor_line];
        end = (g_cursor_line < g_lines_count - 1) ? g_data_lines[g_cursor_line + 1] : g_data_count;
        size = end - start - 1;
        char *clip = malloc(size + 2);
        memcpy(clip, g_data + start, size);
        strcpy(clip + size, "\n");
        clip_set_raw(clip, size + 1);

        if (c == 'c')
            return 0;

        memmove(g_data + start, g_data + end, g_data_count - end);
        g_data_count -= size + 1;

        if (g_lines_count > 1) {
            for (int i = g_cursor_line + 1; i < g_lines_count - 1; i++) {
                g_data_lines[i] = g_data_lines[i + 1];
                g_data_lines[i] -= size + 1;
            }

            g_lines_count--;
        } else {
            g_data_lines[0] = 0;
            g_data_count = 1;
        }

        g_cursor_pos = 0;

        if (g_cursor_line >= g_lines_count) {
            g_cursor_line = g_lines_count - 1;
        }

        realloc_buffer();
    }

    else if (c == 'v') { // paste
        uint32_t size;
        char *clip = clip_get_raw(&size);
        // use insert_newline when '\n' is found
        for (uint32_t i = 0; i < size; i++) {
            if (clip[i] == '\n') {
                insert_newline();
            } else {
                insert_char(clip[i]);
            }
            realloc_buffer();
        }
    }

    else if (c == 'm') { // page down
        g_cursor_line += SCREEN_H;
        if (g_cursor_line >= g_lines_count) {
            g_cursor_line = g_lines_count - 1;
        }
        g_cursor_pos = min(g_cursor_pos, cursor_max_at_line(g_cursor_line));
    }

    else if (c == 'p') { // page up
        g_cursor_line -= SCREEN_H;
        if (g_cursor_line < 0) {
            g_cursor_line = 0;
        }
        g_cursor_pos = min(g_cursor_pos, cursor_max_at_line(g_cursor_line));
    }

    else if (c == 'a') { // home
        g_cursor_pos = 0;
    }

    else if (c == 'e') { // end
        g_cursor_pos = cursor_max_at_line(g_cursor_line);
    }

    return 0;
}

void main_loop(char *path) {
    uint8_t shift_pressed = 0;
    uint8_t ctrl_pressed = 0;

    int last_key = 0, key_sgt = 0;
    int key, key_ticks = 0;

    int y_offset = 0;
    int x_offset = 0;

    int refresh_ticks = 0;

    while (1) {
        // wait for key
        key = syscall_sc_get();

        if (key == 224 || key == 0) {   // RESEND or 0
            key = key_sgt;
        } else {
            key_sgt = key;
        }

        if (key != last_key) key_ticks = 0;
        else key_ticks++;
        last_key = key;

        if ((key_ticks < FIRST_L && key_ticks) || key_ticks % 2) {
            usleep(max((SLEEP_T - refresh_ticks) * 1000, 0));
            refresh_ticks = 0;
            continue;
        }

        // realloc buffer if needed
        realloc_buffer();

        // check if key is enter
        if (key == 28) {
            insert_newline();
        }

        // check if key is escape
        else if (key == 1) {
            return;
        }

        // check if key is ctrl
        else if (key == 29) {
            ctrl_pressed = 1;
        }

        // check if ctrl is released
        else if (key == 157) {
            ctrl_pressed = 0;
        }

        // check if key is ctrl + key
        else if (ctrl_pressed) {
            if (execute_ctrl(key, shift_pressed, path)) {
                return;
            }
        }

        // check if key is backspace
        else if (key == 14) {
            execute_backspace();
        }

        // check if key is delete
        else if (key == 83) {
            if (g_cursor_pos < cursor_max_at_line(g_cursor_line)) {
                for (int i = g_data_lines[g_cursor_line] + g_cursor_pos; i < g_data_count; i++)
                    g_data[i] = g_data[i + 1];

                for (int i = g_cursor_line + 1; i < g_lines_count; i++)
                    g_data_lines[i]--;

                g_data_count--;
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
            insert_tab();
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
            insert_char(profan_kb_get_char(key, shift_pressed));
        } else {
            continue;
        }

        // smart scrolling (y axis)
        if (g_cursor_line - 2 < y_offset) {
            y_offset = max(g_cursor_line - 2, 0);
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
        refresh_ticks = syscall_timer_get_ms();
        display_data(y_offset, min(g_lines_count, y_offset + SCREEN_H), x_offset);
        refresh_ticks = syscall_timer_get_ms() - refresh_ticks;
    }
}

void clear_screen(void) {
    syscall_kprint("\e[2J");
    panda_print_string("\e[2J", 4, -1, 0x0F);
}

void quit(void) {
    free(g_data);
    free(g_data_lines);

    free(g_rim.syntax->keywords);
    free(g_rim.syntax->blues);
    free(g_rim.syntax);
}

char **copy_array(char **array) {
    int len;
    for (len = 0; array[len] != NULL; len++);
    char **copy = malloc((len + 1) * sizeof(char *));
    for (int i = 0; i < len; i++) {
        copy[i] = array[i];
    }
    copy[len] = NULL;
    return copy;
}

void rim_syntax_init(char *lang) {
    g_rim.syntax = calloc(1, sizeof(rim_syntax_t));

    if (!lang)
        return;

    if (strcmp(lang, "c") == 0) {
        g_rim.syntax->words = 1;
        g_rim.syntax->numbers = 1;
        g_rim.syntax->funccalls = 1;
        g_rim.syntax->braces = 1;
        g_rim.syntax->strings = 1;
        g_rim.syntax->ctypes = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "if", "else", "while", "for", "do", "switch", "case",
            "default", "break", "continue", "return", "goto", "end", NULL
        });

        g_rim.syntax->blues = copy_array((char *[]) {
            "char", "short", "int", "long", "float", "double", "void",
            "struct", "enum", "union", "signed", "unsigned", "const",
            "volatile", "static", "extern", "register", "auto", "typedef",
            "bool", "sizeof", "NULL", "true", "false", "inline", "restrict", NULL
        });
    }

    else if (strcmp(lang, "lua") == 0) {
        g_rim.syntax->words = 1;
        g_rim.syntax->numbers = 1;
        g_rim.syntax->funccalls = 1;
        g_rim.syntax->braces = 1;
        g_rim.syntax->strings = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "if", "then", "else", "elseif", "while", "do", "for", "in",
            "repeat", "until", "function", "return", "break", "end", NULL
        });

        g_rim.syntax->blues = copy_array((char *[]) {
            "nil", "true", "false", "function", "local", NULL
        });
    }

    else if (strcmp(lang, "olv") == 0) {
        g_rim.syntax->words = 1;
        g_rim.syntax->numbers = 1;
        g_rim.syntax->strings = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "IF", "ELSE", "WHILE", "FOR", "FUNC", "END", "RETURN", "BREAK", "CONTINUE",
            "if", "else", "while", "for", "func", "end", "return", "break", "continue", NULL
        });
    }
}

char *compute_args(int argc, char **argv) {
    char *file = NULL;
    char *ext = NULL;

    g_rim.always_tab = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                puts("Usage: rim [opt] [file]"
                    "\nOptions:"
                    "\n  -c    specify syntax highlighting"
                    "\n  -h    display this help message"
                    "\n  -n    disable syntax highlighting"
                    "\n  -s    always save file at exit"
                    "\n  -t    always insert tab character"
                    "\n  -v    display version information"
                    "\n\nSyntax highlighting:"
                    "\n  c, lua, olv"
                    "\n\nRim Shortcuts:"
                    "\n  ctrl + q    quit"
                    "\n  ctrl + s    save"
                    "\n  ctrl + c    copy"
                    "\n  ctrl + x    cut"
                    "\n  ctrl + v    paste"
                    "\n  ctrl + m    page down"
                    "\n  ctrl + p    page up"
                    "\n  ctrl + a    home"
                    "\n  ctrl + e    end"
                );
                exit(0);
            } else if (argv[i][1] == 't') {
                g_rim.always_tab = 1;
            } else if (argv[i][1] == 'n') {
                ext = "txt";
            } else if (argv[i][1] == 's') {
                g_rim.save_at_exit = 1;
            } else if (argv[i][1] == 'c') {
                if (i + 1 >= argc) {
                    fprintf(stderr, "rim: Missing argument for option -- 'c'\n");
                    exit(1);
                }
                ext = argv[++i];
            } else if (argv[i][1] == 'v') {
                puts("rim version " RIM_VERSION ", profanOS text editor");
                exit(0);
            } else {
                fprintf(stderr, "rim: Unknown option -- '%s'\n", argv[i] + 1);
                exit(1);
            }
        } else {
            file = argv[i];
        }
    }

    if (ext == NULL && file) {
        ext = strrchr(file, '.');
        if (ext) ext++;
    }

    rim_syntax_init(ext);

    if (!file)
        return NULL;

    file = profan_join_path(profan_wd_path, file);

    return file;
}

int main(int argc, char **argv) {
    char *file = compute_args(argc, argv);
    char *title;

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);
    if (SCREEN_W * SCREEN_H == 0) {
        printf("rim: panda is required\n");
        exit(1);
    }

    g_data_lines = NULL;
    g_data = NULL;

    if (file) {
        title = file;
        load_file(file);
    } else {
        if (isatty(0)) {
            title = "blank -> stdout";
            init_data();
        } else {
            title = "stdin -> stdout";
            load_file(NULL);
        }
    }

    void *old_screen = panda_screen_backup();

    SCREEN_H--;
    SCREEN_W--;

    g_cursor_line = 0;
    g_cursor_pos = 0;

    clear_screen();

    set_title(title);

    display_data(0, min(g_lines_count, SCREEN_H), 0);

    main_loop(file);

    panda_screen_restore(old_screen);
    panda_screen_free(old_screen);

    if (g_rim.save_at_exit || !file)
        save_file(file);
    else
        free(file);

    quit();

    return 0;
}
