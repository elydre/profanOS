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
#include <profan/carp.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define RIM_VERSION "8 rev 1"

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

// interface colors
#define COLOR_IT 0x70   // title
#define COLOR_IL 0x87   // line number
#define COLOR_ID 0x0F   // data
#define COLOR_IM 0x70   // more data (>)
#define COLOR_IU 0x80   // unknown character
#define COLOR_IW 0x08   // whitespace

// syntax highlighting colors
#define COLOR_SD 0x07   // data
#define COLOR_SS 0x06   // string
#define COLOR_SN 0x0A   // number
#define COLOR_ST 0x09   // type
#define COLOR_SK 0x0D   // keyword
#define COLOR_SF 0x0E   // function
#define COLOR_SB 0x0B   // brace

// edit status colors
#define COLOR_EN 0xE0   // unsaved
#define COLOR_ES 0x0A   // saved
#define COLOR_EO 0x07   // original

// character flags
#define RIMCHAR_ORI 0x0100 // present in original file
#define RIMCHAR_NEW 0x0200 // character not saved yet
#define RIMCHAR_SAV 0x0300 // character saved in file
#define RIMCHAR_FTB 0x1000 // fake tab (for displaying)

#define RIMCHAR_SN(c, flag) ((c & 0xF0FF) | (flag))
#define RIMCHAR_ST(c, flag) ((c & 0x0FFF) | (flag))
#define RIMCHAR_CH(c) ((char) (c & 0xFF))
#define RIMCHAR_NF(c) (c & 0x0F00)
#define RIMCHAR_FF(c) (c & 0xF000)

// GLOBALS
typedef struct {
    uint8_t words;
    uint8_t numbers;
    uint8_t funccalls;
    uint8_t braces;
    uint8_t strings;
    uint8_t ctypes;
    uint8_t cpreproc;
    char **keywords;
    char **blues;
} rim_syntax_t;

typedef struct {
    rim_syntax_t *syntax;
    int always_tab;
    int save_at_exit;
    int auto_indent;
    int view;
} rim_settings_t;

enum {
    RIMVIEW_SYNTAX, // syntax highlighting
    RIMVIEW_LITE,   // simple and fast view
    RIMVIEW_EDITS,  // show edits in color
    RIMVIEW_MAX     // number of views
};

rim_settings_t g_rim;
panda_char_t *g_panda_buffer;

uint16_t *g_screen;
uint16_t *g_data;

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

#define set_char_at(x, y, c, color) g_panda_buffer[(y) * (SCREEN_W + 1) + (x)] = (panda_char_t){color, c}

void gui_print(uint32_t x, uint32_t y, char *str, uint16_t color) {
    while (*str) {
        set_char_at(x, y, *str, color);
        x++;
        str++;
    }
}

void set_title(char *path, int unsaved) {
    static int start_pos = 0;

    if (start_pos > 0) {
        if (unsaved)
            set_char_at(start_pos, 0, '*', COLOR_IT);
        else
            set_char_at(start_pos, 0, ' ', COLOR_IT);
        return;
    }

    if (start_pos < 0 || path == NULL)
        return;

    char *title = malloc(SCREEN_W + 1);

    int len = snprintf(title, SCREEN_W, "RIM %s", path);
    memset(title + len, ' ', SCREEN_W - len);

    start_pos = len + 1;

    gui_print(0, 0, title, COLOR_IT);
    free(title);

    if (start_pos > SCREEN_W)
        start_pos = -1;
    else if (unsaved)
        set_char_at(start_pos, 0, '+', COLOR_IT);
}

void init_data(void) {
    free(g_data);
    g_data = malloc(1024 * sizeof(uint16_t));
    g_data[0] = 0;
    g_data_count = 1;
    g_data_max = 1024;

    free(g_data_lines);
    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;
    g_lines_max = 1024;
}

int load_file(char *path) {
    int row, c;
    FILE *f;

    init_data();

    if (path) {
        f = fopen(path, "r");
        if (f == NULL) {
            if (errno == ENOENT)
                return 1; // file does not exist
            fprintf(stderr, "rim: %s: %m\n", path);
            exit(1);
        }
    } else {
        f = stdin;
    }

    g_data_count = row = 0;

    printf("Loading file...\n");

    while ((c = fgetc(f)) != EOF) {
        if (g_data_count >= g_data_max - 5) {
            g_data_max += 1024;
            g_data = realloc(g_data, g_data_max * sizeof(uint16_t));
        }

        if (c == '\n') {
            g_data[g_data_count++] = 0;
            row = 0;

            g_data_lines[g_lines_count] = g_data_count;
            g_lines_count++;

            if (g_lines_count >= g_lines_max) {
                g_lines_max += 1024;
                g_data_lines = realloc(g_data_lines, g_lines_max * sizeof(int));
            }
        } else if (c == '\t') {
            g_data[g_data_count++] = '\t' | RIMCHAR_ORI;
            for (row++; row % 4 != 0; row++)
                g_data[g_data_count++] = '\t' | RIMCHAR_ORI | RIMCHAR_FTB;
        } else {
            g_data[g_data_count++] = c | RIMCHAR_ORI;
            row++;
        }
    }

    printf("Loading file OK\n");

    g_data[g_data_count++] = 0;

    if (f != stdin)
        fclose(f);

    return 0;
}

void save_file(char *path) {
    FILE *f;

    if (path) {
        f = fopen(path, "w+");
        if (f == NULL) {
            fprintf(stderr, "rim: %s: %m\n", path);
            return;
        }
    } else {
        f = stdout;
    }

    for (int i = 0; i < g_data_count - 1; i++) {
        if (g_data[i] & RIMCHAR_NEW)
            g_data[i] = RIMCHAR_SN(g_data[i], RIMCHAR_SAV);

        if (g_data[i] & RIMCHAR_FTB)
            continue; // skip fake tabs

        if (g_data[i] == 0)
            fputc('\n', f);
        else
            fputc(RIMCHAR_CH(g_data[i]), f);
    }

    if (f != stdout)
        fclose(f);
}

int word_isnumber(uint16_t *word, int size) {
    if (!g_rim.syntax->numbers)
        return 0;
    if (size > 2 && RIMCHAR_CH(word[0]) == '0' &&
            (RIMCHAR_CH(word[1]) == 'x' || RIMCHAR_CH(word[1]) == 'X')) {
        for (int i = 2; i < size; i++)
            if (!isxdigit(RIMCHAR_CH(word[i])))
                return 0;
        return 1;
    }
    for (int i = 0; i < size; i++) {
        if (!isdigit(RIMCHAR_CH(word[i])))
            return 0;
    }
    return 1;
}

int word_purple(uint16_t *word, uint32_t size) {
    if (g_rim.syntax->cpreproc && size > 1 && RIMCHAR_CH(word[0]) == '#')
        return 1;

    if (!g_rim.syntax->keywords)
        return 0;

    for (uint32_t i = 0; g_rim.syntax->keywords[i]; i++) {
        // compare word with keyword
        for (uint32_t j = 0; j < size; j++) {
            if (g_rim.syntax->keywords[i][j] == '\0' || g_rim.syntax->keywords[i][j] != RIMCHAR_CH(word[j]))
                break;
            if (g_rim.syntax->keywords[i][j + 1] == '\0' && j + 1 == size)
                return 1;
        }
    }
    return 0;
}

int word_isblue(uint16_t *word, uint32_t size) {
    if (g_rim.syntax->ctypes && size > 2 && RIMCHAR_CH(word[size - 1]) == 't' && RIMCHAR_CH(word[size - 2]) == '_')
        return 1;
    if (!g_rim.syntax->blues)
        return 0;
    for (uint32_t i = 0; g_rim.syntax->blues[i]; i++) {
        // compare word with blue
        for (uint32_t j = 0; j < size; j++) {
            if (g_rim.syntax->blues[i][j] == '\0' || g_rim.syntax->blues[i][j] != RIMCHAR_CH(word[j]))
                break;
            if (g_rim.syntax->blues[i][j + 1] == '\0' && j + 1 == size)
                return 1;
        }
    }
    return 0;
}

int word_isbrace(uint16_t *word, uint32_t size) {
    if (!g_rim.syntax->braces || size != 1)
        return 0;

    switch (RIMCHAR_CH(*word)) {
        case '{':
        case '}':
        case '(':
        case ')':
        case '[':
        case ']':
            return 1;
        default:
            return 0;
    }
}

int word_paraft(uint16_t *word, uint32_t size) {
    if (!g_rim.syntax->funccalls)
        return 0;
    for (uint32_t i = size; word[i]; i++) {
        if (RIMCHAR_CH(word[i]) == '(')
            return 1;
        if (!isspace(RIMCHAR_CH(word[i])))
            return 0;
    }
    return 0;
}

void put_word(int line, int in_word, uint16_t *g_screen, int g_screen_i, uint16_t *word, int size) {
    if (size == 0)
        return;

    char color = COLOR_ID;

    if (in_word == 2 && g_rim.syntax->strings)
        color = COLOR_SS;

    else if (in_word == 1 && g_rim.syntax->words) {
        color = COLOR_SD;
        if (word_isnumber(word, size))
            color = COLOR_SN;
        else if (word_isblue(word, size))
            color = COLOR_ST;
        else if (word_purple(word, size))
            color = COLOR_SK;
        else if (word_paraft(word, size))
            color = COLOR_SF;
    }

    else if (word_isbrace(word, size))
        color = COLOR_SB;

    if (line > g_screen_i) {
        size = size - (line - g_screen_i);
        if (size <= 0)
            return;
        word += line - g_screen_i;
        g_screen_i = line;
    }

    for (int k = 0; k < size; k++) {
        g_screen[g_screen_i + k] = RIMCHAR_CH(word[k]) | (color << 8);
    }
}

#define localput_word(X) put_word((i - from_line) * SCREEN_W, in_word, g_screen, \
    line * SCREEN_W + world_start - x_offset, g_data + g_data_lines[i] + world_start, (X))

#define RIM_IS_WORD_CHAR(c) (isalnum(c) || c == '_' || c == '$' || (g_rim.syntax->cpreproc && c == '#'))

void calc_screen_color(int from_line, int to_line, int x_offset) {
    int line = 0;

    int world_start, in_word;
    char chr_str;

    for (int i = from_line; i < to_line; i++) {
        in_word = world_start = 0;

        int max = cursor_max_at_line(i);

        // out-screen characters (x_offset)
        for (int j = 0; j < x_offset && j < max; j++) {
            char c = g_data[g_data_lines[i] + j];

            if (in_word == 2 && c == chr_str)
                in_word = 0;

            else if (c == '\"' || c == '\'') {
                chr_str = c;
                in_word = 2;
            }

            if (in_word == 2)
                continue;

            if (RIM_IS_WORD_CHAR(c)) {
                if (!in_word) {
                    in_word = 1;
                    world_start = j;
                }
            } else {
                in_word = 0;
            }
        }

        // in-screen characters
        for (int j = x_offset;; j++) {
            if (j >= max) {
                localput_word(j - world_start);
                break;
            }

            if (j >= x_offset + SCREEN_W) {
                localput_word(j - (world_start));
                break;
            }

            char c = RIMCHAR_CH(g_data[g_data_lines[i] + j]);

            if (c == '\t') {
                g_screen[line * SCREEN_W + j - x_offset] = '>' | (COLOR_IW << 8);
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2)
                    in_word = 0;
                continue;
            }

            if (!isprint(c)) {
                g_screen[line * SCREEN_W + j - x_offset] = (c ? '?' : '0') | (COLOR_IU << 8);
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2)
                    in_word = 0;
                continue;
            }

            else if (c == ' ') {
                g_screen[line * SCREEN_W + j - x_offset] = '.' | (COLOR_IW << 8);
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2)
                    in_word = 0;
                continue;
            }

            if (in_word == 2) {
                if (c == chr_str) {
                    localput_word(j - world_start + 1);
                    world_start = j + 1;
                    in_word = 0;
                }
                continue;
            }

            else if (c == '\"' || c == '\'') {
                localput_word(j - world_start);
                chr_str = c;
                world_start = j;
                in_word = 2;
            }

            else if (in_word != 2) {
                if (RIM_IS_WORD_CHAR(c)) {
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

        line++;
    }
}

void calc_screen_lite(int from_line, int to_line, int x_offset) {
    int max, line = 0;

    for (int i = from_line; i < to_line; i++) {
        max = cursor_max_at_line(i);

        // in-screen characters
        for (int j = x_offset; j < max; j++) {
            if (j >= x_offset + SCREEN_W)
                break;

            uint16_t lc = g_data[g_data_lines[i] + j];
            char c = RIMCHAR_CH(lc);

            lc = (g_rim.view == RIMVIEW_EDITS ? (
                    RIMCHAR_NF(lc) == RIMCHAR_NEW ? COLOR_EN :
                    RIMCHAR_NF(lc) == RIMCHAR_SAV ? COLOR_ES :
                    RIMCHAR_NF(lc) == RIMCHAR_ORI ? COLOR_EO :
                    COLOR_IU) : COLOR_ID) << 8;
            if (c == '\t')
                c = ' ';

            if (!isprint(c))
                g_screen[line * SCREEN_W + j - x_offset] = '?' | (COLOR_IU << 8);
            else {
                g_screen[line * SCREEN_W + j - x_offset] = c | lc;
            }
        }
        line++;
    }
}

void display_data(int from_line, int to_line, int x_offset) {
    if (to_line - from_line > SCREEN_H) {
        fputs("rim: error: too many lines to display\n", stderr);
        exit(1);
    }

    memset(g_screen, 0, SCREEN_W * SCREEN_H * sizeof(uint16_t));

    if (g_rim.view == RIMVIEW_SYNTAX)
        calc_screen_color(from_line, to_line, x_offset);
    else
        calc_screen_lite(from_line, to_line, x_offset);

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
        for (int j = 0; j < SCREEN_W - line_offset; j++) {
            pos = i * SCREEN_W + j;
            if (g_screen[pos] == 0) {
                set_char_at(j + line_offset + 1, y, ' ', COLOR_ID);
            } else if (j == SCREEN_W - line_offset - 1) {
                set_char_at(j + line_offset + 1, y, '>', COLOR_IM);
            } else {
                set_char_at(j + line_offset + 1, y, g_screen[pos] & 0xFF, g_screen[pos] >> 8);
            }
        }

        if (i < to_line - from_line) {
            // line number
            itoa(from_line + i + 1, line_str + 2, 10);
            gui_print(0, y, line_str + strlen(line_str) - line_offset, COLOR_IL);
            set_char_at(line_offset, y, ' ', COLOR_ID);
        }

        y++;
    }

    for (int i = y; i <= SCREEN_H; i++) {
        for (int j = line_offset; j < SCREEN_W; j++)
            set_char_at(j, i, ' ', COLOR_ID);
        for (int j = 0; j < line_offset; j++)
            set_char_at(j, i, ' ', COLOR_IL);
        set_char_at(line_offset, i, ' ', COLOR_ID);
    }

    // cursor
    panda_print_raw(g_panda_buffer, (SCREEN_W + 1) * (SCREEN_H + 1));
    panda_draw_cursor(g_cursor_pos - x_offset + line_offset + 1, g_cursor_line - from_line + 1);
}

void realloc_buffer(void) {
    if (g_lines_count >= g_lines_max - 2) {
        if (g_lines_count > g_lines_max) {
            fputs("rim: g_data_lines: overflow detected\n", stderr);
            exit(1);
        }
        g_lines_max += 1024;
        g_data_lines = realloc(g_data_lines, g_lines_max * sizeof(int));
    }

    if (g_data_count >= g_data_max - 2) {
        if (g_data_count > g_data_max) {
            fputs("rim: g_data: overflow detected\n", stderr);
            exit(1);
        }
        g_data_max += 1024;
        g_data = realloc(g_data, g_data_max * sizeof(uint16_t));
    }
}

void reallign_tabs(void) {
    int tab, new_len, original_len;
    tab = new_len = 0;
    original_len = 1;

    for (int i = g_data_lines[g_cursor_line]; g_data[i]; i++) {
        original_len++;

        if (g_data[i] & RIMCHAR_FTB) {
            if (tab == 0)
                tab++;
            continue;
        }

        if (RIMCHAR_CH(g_data[i]) == '\t')
            tab++;

        new_len++;
    }

    if (tab == 0)
        return;

    uint16_t *new_line = malloc((new_len + tab * 3 + 1000) * sizeof(uint16_t));

    // copy data to new line and add fake tabs
    int j = 0;
    for (int i = g_data_lines[g_cursor_line]; g_data[i] != 0; i++) {
        if (g_data[i] & RIMCHAR_FTB)
            continue;
        new_line[j++] = g_data[i];
        if (RIMCHAR_CH(g_data[i]) == '\t') {
            int flag = RIMCHAR_NF(g_data[i]);
            for (; j % 4 != 0; j++)
                new_line[j] = '\t' | flag | RIMCHAR_FTB;
        }
    }

    new_line[j++] = 0;

    int offset = j - original_len;
    original_len += g_data_lines[g_cursor_line];

    // realloc buffer if needed
    while (g_data_count + offset >= g_data_max) {
        g_data_max += 1024;
        g_data = realloc(g_data, g_data_max * sizeof(uint16_t));
    }

    // move data after current line
    if (offset) {
        memmove(g_data + g_data_lines[g_cursor_line] + j, g_data + original_len,
                (g_data_count - original_len) * sizeof(uint16_t));
    }

    // copy new line to data buffer
    memcpy(g_data + g_data_lines[g_cursor_line], new_line, j * sizeof(uint16_t));
    free(new_line);

    // update data lines
    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i] += offset;

    g_data_count += offset;
}

void finish_edits(void) {
    reallign_tabs();
    realloc_buffer();
    set_title(NULL, 1);
}

void cursor_tab_allign(int right) {
    // jump over the fake tabs
    while (g_data[g_data_lines[g_cursor_line] + g_cursor_pos] & RIMCHAR_FTB && g_cursor_pos > 0) {
        if (right)
            g_cursor_pos++;
        else
            g_cursor_pos--;
    }
}

void insert_char(char chr) {
    // add character to data buffer
    for (int i = g_data_count; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
        g_data[i] = g_data[i - 1];

    g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = RIMCHAR_SN(chr, RIMCHAR_NEW);
    g_data_count++;

    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i]++;

    g_cursor_pos++;
}

void insert_tab(int force) {
    if (g_rim.always_tab || force) {
        goto REAL_TAB;
    } else for (int j = g_data_lines[g_cursor_line]; g_data[j] != 0; j++) {
        if (g_data[j] == '\t')
            goto REAL_TAB;
    }

    int spaces = 4 - (g_cursor_pos % 4);
    // realloc buffer if needed
    if (g_data_count + spaces + 2 >= g_data_max) {
        g_data_max += 1024;
        g_data = realloc(g_data, g_data_max * sizeof(uint16_t));
    }

    // add character to data buffer
    for (int i = g_data_count + spaces - 1; i >= spaces && i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
        g_data[i] = g_data[i - spaces];

    for (int i = 0; i < spaces; i++)
        g_data[g_data_lines[g_cursor_line] + g_cursor_pos + i] = RIMCHAR_ST(' ', RIMCHAR_NEW);

    g_data_count += spaces;

    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i] += spaces;

    g_cursor_pos += spaces;

    finish_edits();
    return;

    REAL_TAB:
    insert_char('\t');
    finish_edits();
    cursor_tab_allign(1);
}

void execute_delete(void) {
    if (g_cursor_pos >= cursor_max_at_line(g_cursor_line))
        return;
    for (int i = g_data_lines[g_cursor_line] + g_cursor_pos; i < g_data_count; i++)
        g_data[i] = g_data[i + 1];

    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i]--;

    g_data_count--;
    finish_edits();
}

void insert_newline(int auto_indent) {
    int spaces = 0;

    if (auto_indent) {
        int base = g_data_lines[g_cursor_line];

        // count spaces in current line
        for (int i = base; i < g_cursor_pos + base &&
                (RIMCHAR_CH(g_data[i]) == ' ' || RIMCHAR_CH(g_data[i]) == '\t'); i++)
            spaces++;

        // trim the end of the line
        while (g_cursor_pos > 0 && (RIMCHAR_CH(g_data[base + g_cursor_pos - 1]) == ' '
                || RIMCHAR_CH(g_data[base + g_cursor_pos - 1]) == '\t')) {
            g_cursor_pos--;
            execute_delete();
        }
    }

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

    if (!auto_indent || spaces == 0)
        return;

    while (spaces >= 4) {
        insert_tab(0);
        spaces -= 4;
    }

    while (spaces > 0) {
        insert_char(' ');
        spaces--;
    }
}

void execute_backspace(void) {
    if (g_cursor_pos > 0) {
        g_cursor_pos--;
        cursor_tab_allign(0);
        execute_delete();
        return;
    }

    if (g_cursor_line <= 0)
        return;

    int future_cursor_pos = cursor_max_at_line(g_cursor_line - 1);

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

    finish_edits();
}

int execute_ctrl(int key, uint8_t shift, char *path) {
    int start, end, size;

    char c = profan_kb_get_char(key, shift);

    if (c == 'q') { // quit
        return 1;
    }

    if (c == 's' && path) { // save
        save_file(path);
        set_title(NULL, 0);
    }

    else if (c == 'c' || c == 'x') { // copy or cut
        start = g_data_lines[g_cursor_line];
        end = (g_cursor_line < g_lines_count - 1) ? g_data_lines[g_cursor_line + 1] : g_data_count;
        size = 0;
        char *clip = malloc(end - start + 1);
        for (int i = start; i < end - 1; i++) {
            if (g_data[i] & RIMCHAR_FTB)
                continue; // skip fake tabs
            clip[size++] = RIMCHAR_CH(g_data[i]);
        }
        clip[size++] = '\n';
        clip[size] = '\0';
        clip_set_raw(clip, size);
        free(clip);

        if (c == 'c')
            return 0;

        memmove(g_data + start, g_data + end, (g_data_count - end) * sizeof(uint16_t));
        g_data_count -= size;

        if (g_lines_count > 1) {
            for (int i = g_cursor_line + 1; i < g_lines_count - 1; i++) {
                g_data_lines[i] = g_data_lines[i + 1];
                g_data_lines[i] -= size;
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
        finish_edits();
    }

    else if (c == 'v') { // paste
        uint32_t size;
        char *clip = clip_get_raw(&size);
        // use insert_newline when '\n' is found
        for (uint32_t i = 0; i < size; i++) {
            if (clip[i] == '\t') {
                insert_tab(1);
            } else if (clip[i] == '\n') {
                insert_newline(0);
            } else {
                insert_char(clip[i]);
            }
            realloc_buffer();
        }
        free(clip);
        finish_edits();
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

    else if (c == 'n') { // switch view
        g_rim.view = (g_rim.view + 1) % RIMVIEW_MAX;
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

        // check if key is enter
        if (key == 28) {
            insert_newline(g_rim.auto_indent);
            finish_edits();
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
            execute_delete();
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
            insert_tab(shift_pressed);
        }

        // check if key is arrow left
        else if (key == 75) {
            // move cursor
            if (g_cursor_pos > 0) {
                g_cursor_pos--;
            }
            cursor_tab_allign(0);
        }

        // check if key is arrow right
        else if (key == 77) {
            // move cursor
            if (g_cursor_pos < cursor_max_at_line(g_cursor_line)) {
                g_cursor_pos++;
            }
            cursor_tab_allign(1);
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
            cursor_tab_allign(0);
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
            cursor_tab_allign(0);
        }

        // check if key is printable
        else if (key < 58 && key > 0 && profan_kb_get_char(key, shift_pressed)) {
            insert_char(profan_kb_get_char(key, shift_pressed));
            finish_edits();
        }

        // nothing to do
        else continue;

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
    (void) panda_print_string("\e[2J", 4, -1, 0x0F);
}

void quit(void) {
    free(g_panda_buffer);
    free(g_data_lines);
    free(g_screen);
    free(g_data);

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

void rim_syntax_init(const char *lang) {
    g_rim.syntax = calloc(1, sizeof(rim_syntax_t));
    g_rim.syntax->words = 1;

    if (!lang)
        return;

    if (strcmp(lang, "c") == 0) {
        g_rim.syntax->numbers = 1;
        g_rim.syntax->funccalls = 1;
        g_rim.syntax->braces = 1;
        g_rim.syntax->strings = 1;
        g_rim.syntax->ctypes = 1;
        g_rim.syntax->cpreproc = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "if", "else", "while", "for", "do", "switch", "case",
            "default", "break", "continue", "return", "goto", NULL
        });

        g_rim.syntax->blues = copy_array((char *[]) {
            "char", "short", "int", "long", "float", "double", "void",
            "struct", "enum", "union", "signed", "unsigned", "const",
            "volatile", "static", "extern", "register", "auto", "typedef",
            "bool", "sizeof", "NULL", "true", "false", "inline", "restrict", NULL
        });
    }

    else if (strcmp(lang, "lua") == 0) {
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

    else if (strcmp(lang, "py") == 0) {
        g_rim.syntax->numbers = 1;
        g_rim.syntax->funccalls = 1;
        g_rim.syntax->braces = 1;
        g_rim.syntax->strings = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "as", "assert", "async", "await", "break", "class", "continue", "del",
            "elif", "else", "except", "finally", "for", "from", "if", "import",
            "in", "pass", "raise", "return", "try", "while", "with", "yield", NULL
        });

        g_rim.syntax->blues = copy_array((char *[]) {
            "False", "None", "True", "and", "def", "global", "is", "lambda",
            "nonlocal", "not", "or", NULL
        });
    }

    else if (strcmp(lang, "olv") == 0) {
        g_rim.syntax->numbers = 1;
        g_rim.syntax->strings = 1;

        g_rim.syntax->keywords = copy_array((char *[]) {
            "IF", "ELSE", "WHILE", "FOR", "FUNC", "END", "RETURN", "BREAK", "CONTINUE",
            "if", "else", "while", "for", "func", "end", "return", "break", "continue", NULL
        });
    }
}

char *compute_args(int argc, char **argv) {
    carp_init("[options] [file]", 1);
    carp_set_ver("rim", RIM_VERSION);

    carp_register('c', CARP_NEXT_STR, "specify syntax highlighting");
    carp_register('i', CARP_STANDARD, "disable automatic indentation");
    carp_register('M', CARP_STANDARD, "show a memo of keyboard shortcuts");
    carp_register('s', CARP_STANDARD, "always save file at exit");
    carp_register('t', CARP_STANDARD, "use tabs instead of spaces");

    if (carp_parse(argc, argv))
        exit(1);

    const char *ext = NULL;

    memset(&g_rim, 0, sizeof(g_rim));

    if (carp_isset('M')) {
        puts("Rim Shortcuts:\n"
            "   ctrl + q     quit\n"
            "   ctrl + s     save\n"
            "   ctrl + c     copy\n"
            "   ctrl + x     cut\n"
            "   ctrl + v     paste\n"
            "   ctrl + m     page down\n"
            "   ctrl + p     page up\n"
            "   ctrl + a     home\n"
            "   ctrl + e     end\n"
            "   ctrl + n     switch view\n"
            "  shift + tab   insert \\t\n\n"
            "Syntax highlighting:\n"
            "  c, lua, py, olv"
        );

        exit(0);
    }

    if (carp_isset('t'))
        g_rim.always_tab = 1;
    if (carp_isset('s'))
        g_rim.save_at_exit = 1;
    if (carp_isset('c'))
        ext = carp_get_str('c');
    if (!carp_isset('i'))
        g_rim.auto_indent = 1;

    const char *file = carp_file_next();

    if (ext == NULL && file) {
        ext = strrchr(file, '.');
        if (ext) ext++;
    }

    rim_syntax_init(ext);

    if (!file)
        return NULL;

    return profan_path_join(profan_wd_path(), file);
}

int main(int argc, char **argv) {
    char *title, *file = compute_args(argc, argv);
    int is_new = 0;

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);

    if (SCREEN_W * SCREEN_H == 0) {
        printf("rim: panda is required\n");
        exit(1);
    }

    g_panda_buffer = malloc(SCREEN_W * SCREEN_H * sizeof(panda_char_t));
    g_data_lines = NULL;
    g_data = NULL;

    if (file) {
        title = file;
        is_new = load_file(file);
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

    g_screen = malloc(SCREEN_H * SCREEN_W * sizeof(uint16_t));

    g_cursor_line = 0;
    g_cursor_pos = 0;

    clear_screen();

    set_title(title, is_new);

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
