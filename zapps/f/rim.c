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
#include <modules/panda.h>
#include <profan/clip.h>
#include <profan/carp.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <ctype.h>

#define RIM_VERSION "9.0"

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

#define cursor_max_at_line(line) (((line) < g_lines_count - 1) ? g_data_lines[(line) + 1] \
            - g_data_lines[line] - 1 :  g_data_count - g_data_lines[line] - 1)

// interface colors

#define COLOR_BT  0x111111   // background
#define COLOR_BC  0x2a2a2a   // background cursor

#define COLOR_FIT 0x999999   // title
#define COLOR_BIT 0x333333   // title background

#define COLOR_FLU 0x888888   // line number
#define COLOR_BLU 0x000000   // line number background

#define COLOR_FLC 0xe972ed   // line number cursor
#define COLOR_BLC 0x221a22   // line number background cursor

#define COLOR_FIM 0x0000FF   // more data (>)
#define COLOR_BIM 0x000000   // more data background

#define COLOR_FIU 0xFF0000   // unknown character (?)
#define COLOR_BIU 0x000000   // unknown character background

#define COLOR_ORI COLOR_BT  // original line
#define COLOR_SAV 0x63ae68  // saved line
#define COLOR_NEW 0xd1b541  // edited line

// syntax highlighting colors
#define COLOR_SD 0xE0E0E0   // data
#define COLOR_SO 0xBBBBBB   // word
#define COLOR_SS 0xf69e3f   // string
#define COLOR_SN 0x5AF78E   // number
#define COLOR_ST 0x3a65ff   // type
#define COLOR_SK 0xf543ff   // keyword
#define COLOR_SF 0xe6e760   // function
#define COLOR_SB 0xFFFFFF   // brace
#define COLOR_SW 0x3B3B3B   // whitespace
#define COLOR_SC 0x42981b   // comment

// character flags
#define RIMCHAR_ORI 0x0100 // present in original file
#define RIMCHAR_SAV 0x0200 // character saved in file
#define RIMCHAR_NEW 0x0400 // character not saved yet
#define RIMCHAR_FTB 0x1000 // fake tab (for displaying)
#define RIMCHAR_NWL 0x2000 // new line

#define RIMCHAR_SN(c, flag) ((c & 0xFFFFFFF0FF) | (flag))
#define RIMCHAR_ST(c, flag) ((c & 0xFFFFFF0FFF) | (flag))
#define RIMCHAR_CH(c) ((char) (c & 0xFF))
#define RIMCHAR_NF(c) (c & 0x0F00)
#define RIMCHAR_FF(c) (c & 0xF000)
#define RIMCHAR_DT(c) (c & 0x0000FFFF)
#define RIMCHAR_CL(c) ((c & 0xFFFF0000) >> 16)

#define HALF_HALF_CHAR '\003'

enum {
    TYPE_TEXT,
    TYPE_WORD,
    TYPE_NUMBER,
    TYPE_FUNCCALL,
    TYPE_BRACE,
    TYPE_STRING,
    TYPE_PURPLE,
    TYPE_BLUE,
    TYPE_COMMENT
};

// GLOBALS
typedef struct {
    uint8_t words;
    uint8_t numbers;
    uint8_t funccalls;
    uint8_t braces;
    uint8_t strings;
    uint8_t ctypes;
    uint8_t cpreproc;
    uint8_t dslcom;     // double slash comments
    uint8_t scpcom;     // script comments (#)
    uint8_t mltcom;     // multi-line comments (/* */)
    char **keywords;
    char **blues;
} rim_syntax_t;

typedef struct {
    rim_syntax_t *syntax;
    int always_tab;
    int save_at_exit;
    int auto_indent;
} rim_settings_t;

rim_settings_t g_rim;
panda_char_t *g_panda_buffer;

uint32_t *g_data;

int g_data_count;
int g_data_max;

int *g_data_lines;
int g_lines_count;
int g_lines_max;

int g_cursor_line;
int g_cursor_pos;

int SCREEN_W;
int SCREEN_H;

/******************************************
 *                                       *
 *         INTERFACE DISPLAYING          *
 *                                       *
******************************************/

#define set_charat(x, y, c, fg, bg) g_panda_buffer[(y) * (SCREEN_W + 1) + (x)] = \
            (panda_char_t){.fg_color = (fg), .bg_color = (bg), .decoration = 0, .character = c}

#define set_charat_underline(x, y, c, fg, bg) g_panda_buffer[(y) * (SCREEN_W + 1) + (x)] = \
            (panda_char_t){.fg_color = (fg), .bg_color = (bg), .decoration = PANDA_UNDERLINE, .character = c}

void gui_print(uint32_t x, uint32_t y, char *str, uint32_t fg, uint32_t bg) {
    while (*str) {
        set_charat(x, y, *str, fg, bg);
        x++;
        str++;
    }
}

void set_title(char *path, int unsaved) {
    static int start_pos = 0;

    if (start_pos > 0) {
        if (unsaved)
            set_charat_underline(start_pos, 0, '*', COLOR_FIT, COLOR_BIT);
        else
            set_charat_underline(start_pos, 0, ' ', COLOR_FIT, COLOR_BIT);
        return;
    }

    if (start_pos < 0 || path == NULL)
        return;

    char *title = malloc(SCREEN_W + 1);

    int len = snprintf(title, SCREEN_W, "RIM %s", path);
    memset(title + len, ' ', SCREEN_W - len);

    start_pos = len + 1;

    for (int i = 0; i < SCREEN_W; i++)
        set_charat_underline(i, 0, title[i], COLOR_FIT, COLOR_BIT);
    free(title);

    if (start_pos > SCREEN_W)
        start_pos = -1;
    else if (unsaved)
        set_charat_underline(start_pos, 0, '+', COLOR_FIT, COLOR_BIT);
}

/******************************************
 *                                       *
 *           LOADING FONCTIONS           *
 *                                       *
******************************************/

void init_data(void) {
    free(g_data);
    g_data = malloc(1024 * sizeof(uint32_t));
    g_data[0] = RIMCHAR_ORI;
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

    while ((c = fgetc(f)) != EOF) {
        if (g_data_count >= g_data_max - 5) {
            g_data_max += 1024;
            g_data = realloc(g_data, g_data_max * sizeof(uint32_t));
        }

        if (c == '\n') {
            g_data[g_data_count++] = RIMCHAR_NWL | RIMCHAR_ORI;
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

    g_data[g_data_count++] = RIMCHAR_ORI;

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

    for (int i = 0;; i++) {
        if (g_data[i] & RIMCHAR_NEW)
            g_data[i] = RIMCHAR_SN(g_data[i], RIMCHAR_SAV);

        if (i == g_data_count - 1)
            break;

        if (g_data[i] & RIMCHAR_FTB)
            continue; // skip fake tabs

        if (RIMCHAR_FF(g_data[i]) & RIMCHAR_NWL)
            fputc('\n', f);
        else
            fputc(RIMCHAR_CH(g_data[i]), f);
    }

    if (f != stdout)
        fclose(f);
}

/******************************************
 *                                       *
 *          SYNTAX HIGHLIGHTING          *
 *                                       *
******************************************/

int word_isnumber(uint32_t *word, int size) {
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

int word_purple(uint32_t *word, uint32_t size) {
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

int word_isblue(uint32_t *word, uint32_t size) {
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

int word_isbrace(uint32_t *word, uint32_t size) {
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

int word_paraft(uint32_t *word, uint32_t size) {
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

void put_word(int in_word, uint32_t *word, int size) {
    if (size == 0)
        return;

    char color = TYPE_TEXT;

    if (in_word == 2 && g_rim.syntax->strings)
        color = TYPE_STRING;

    else if (in_word == 3)
        color = TYPE_COMMENT;

    else if (in_word == 1 && g_rim.syntax->words) {
        color = TYPE_WORD;
        if (word_isnumber(word, size))
            color = TYPE_NUMBER;
        else if (word_isblue(word, size))
            color = TYPE_BLUE;
        else if (word_purple(word, size))
            color = TYPE_PURPLE;
        else if (word_paraft(word, size))
            color = TYPE_FUNCCALL;
    }

    else if (word_isbrace(word, size))
        color = TYPE_BRACE;

    for (int i = 0; i < size; i++) {
        word[i] = RIMCHAR_DT(word[i]) | color << 16;
    }
}

#define localput_word(X) put_word(in_word, g_data + g_data_lines[i] + world_start, (X))

#define RIM_IS_WORD_CHAR(c) (isalnum(c) || c == '_' || c == '$' || (g_rim.syntax->cpreproc && c == '#'))

void color_lines(int from_line, int to_line) {
    int world_start, in_word = 0;
    char chr_str;

    for (int i = from_line; i < to_line; i++) {
        world_start = 0;
        if (in_word != 3)
            in_word = 0;

        int max = cursor_max_at_line(i);

        // in-screen characters
        for (int j = 0;; j++) {
            if (j >= max) {
                localput_word(j - world_start);
                break;
            }

            char c = RIMCHAR_CH(g_data[g_data_lines[i] + j]);
            char c_next = (j + 1 < max) ? RIMCHAR_CH(g_data[g_data_lines[i] + j + 1]) : 0;

            if (g_rim.syntax->mltcom && in_word < 2 && c == '/' && c_next == '*') {
                localput_word(j - world_start);
                world_start = j;
                in_word = 3;
            }

            else if (in_word == 3 && c == '*' && c_next == '/') {
                localput_word(j - world_start + 2);
                world_start = j + 2;
                in_word = 0;
                j++; // skip next '/'
            }

            else if (in_word == 3) {
                if (j == max - 1) {
                    localput_word(j - world_start + 1);
                    break;
                }
            }

            else if ((g_rim.syntax->dslcom && in_word < 2 && c == '/' && c_next == '/') ||
                        (g_rim.syntax->scpcom && !in_word && c == '#')) {
                localput_word(j - world_start);
                put_word(3, g_data + g_data_lines[i] + j, max - j);
                world_start = j;
                break;
            }

            else if (c == ' ' || !isprint(c)) {
                localput_word(j - world_start);
                world_start = j + 1;
                if (in_word != 2)
                    in_word = 0;
            }

            else if (in_word == 2) {
                if (c == chr_str) {
                    localput_word(j - world_start + 1);
                    world_start = j + 1;
                    in_word = 0;
                }
            }

            else if (c == '\"' || c == '\'') {
                localput_word(j - world_start);
                chr_str = c;
                world_start = j;
                in_word = 2;
            }

            else if (in_word != 2) {
                if (RIM_IS_WORD_CHAR(c)) {
                    if (in_word)
                        continue;
                    localput_word(j - world_start);
                    world_start = j;
                    in_word = 1;
                } else {
                    localput_word(j - world_start);
                    world_start = j;
                    in_word = 0;
                }
            }
        }
    }
}

/******************************************
 *                                       *
 *       TEXT DISPLAYING FONCTIONS       *
 *                                       *
******************************************/

void display_data(int from_line, int to_line, int x_offset) {
    if (to_line - from_line > SCREEN_H) {
        fputs("rim: error: too many lines to display\n", stderr);
        exit(1);
    }

    int y = 1;
    char line_str[15];
    memset(line_str, ' ', sizeof(line_str));

    profan_itoa(g_lines_count, line_str + 5, 10);
    int line_offset = strlen(line_str) - 4;
    if (line_offset < 4)
        line_offset = 4;

    for (int i = from_line; i < to_line; i++) {
        int max = cursor_max_at_line(i);

        uint32_t status = RIMCHAR_NF(g_data[g_data_lines[i] + max]);

        if (i == g_cursor_line)
            set_charat(line_offset, y, HALF_HALF_CHAR, COLOR_BLC, COLOR_BC);
        else
            set_charat(line_offset, y, HALF_HALF_CHAR, COLOR_BLU, COLOR_BT);

        // line content
        for (int j = 0; j < SCREEN_W - line_offset; j++) {
            int col = j + x_offset;

            uint32_t fg = COLOR_SD;
            uint32_t bg = (i == g_cursor_line) ? COLOR_BC : COLOR_BT;

            if (col >= max) {
                set_charat(j + line_offset + 1, y, ' ', fg, bg);
                continue;
            }

            uint32_t lc = g_data[g_data_lines[i] + col];
            char c = RIMCHAR_CH(lc);

            if (c == '\t') {
                c = '>';
                fg = COLOR_SW;
            } else if (c == ' ') {
                c = '.';
                fg = COLOR_SW;
            } else if (!isprint(c)) {
                c = c ? '?' : '0';
                fg = COLOR_FIU;
                bg = COLOR_BIU;
            } else switch (RIMCHAR_CL(lc)) {
                case TYPE_TEXT:     fg = COLOR_SD; break;
                case TYPE_WORD:     fg = COLOR_SO; break;
                case TYPE_NUMBER:   fg = COLOR_SN; break;
                case TYPE_STRING:   fg = COLOR_SS; break;
                case TYPE_BRACE:    fg = COLOR_SB; break;
                case TYPE_FUNCCALL: fg = COLOR_SF; break;
                case TYPE_PURPLE:   fg = COLOR_SK; break;
                case TYPE_BLUE:     fg = COLOR_ST; break;
                case TYPE_COMMENT:  fg = COLOR_SC; break;
            }

            if (RIMCHAR_NF(lc) > status) {
                status = RIMCHAR_NF(lc);
            }

            // line going out of screen
            if (j == SCREEN_W - line_offset - 1) {
                c = '>';
                fg = COLOR_FIM;
                bg = COLOR_BIM;
            }

            set_charat(j + line_offset + 1, y, c, fg, bg);
        }

        // line number
        if (i < to_line) {
            profan_itoa(i + 1, line_str + 5, 10);

            gui_print(1, y, line_str + strlen(line_str) - line_offset + 1,
                (i == g_cursor_line) ? COLOR_FLC : COLOR_FLU,
                (i == g_cursor_line) ? COLOR_BLC : COLOR_BLU);

            set_charat(0, y, HALF_HALF_CHAR,
                (status == RIMCHAR_ORI) ? COLOR_ORI :
                (status == RIMCHAR_SAV) ? COLOR_SAV :
                (status == RIMCHAR_NEW) ? COLOR_NEW :
                (status == 0) ? 0xFFFFFF :
                COLOR_FIU,
                (i == g_cursor_line) ? COLOR_BLC : COLOR_BLU);
        }

        y++;
    }

    // clear rest of screen
    for (int i = y; i <= SCREEN_H; i++) {
        set_charat(line_offset, i, HALF_HALF_CHAR, COLOR_BLU, COLOR_BT);
        set_charat(0, i, HALF_HALF_CHAR, COLOR_ORI, COLOR_BLU);

        for (int j = 1; j < line_offset; j++)
            set_charat(j, i, ' ', COLOR_FLU, COLOR_BLU);
        for (int j = line_offset + 1; j <= SCREEN_W; j++)
            set_charat(j, i, ' ', COLOR_SD, COLOR_BT);
    }

    panda_print_raw(g_panda_buffer, (SCREEN_W + 1) * (SCREEN_H + 1));
    panda_draw_cursor(g_cursor_pos - x_offset + line_offset + 1,
                      g_cursor_line - from_line + 1);
}

/******************************************
 *                                       *
 *           EDITING FONCTIONS           *
 *                                       *
******************************************/

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
        g_data = realloc(g_data, g_data_max * sizeof(uint32_t));
    }
}

void reallign_tabs(void) {
    int tab, new_len, original_len;
    tab = new_len = 0;
    original_len = 1;

    for (int i = g_data_lines[g_cursor_line]; RIMCHAR_CH(g_data[i]); i++) {
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

    uint32_t *new_line = malloc((new_len + tab * 3 + 1000) * sizeof(uint32_t));

    // copy data to new line and add fake tabs
    int j = 0;
    for (int i = g_data_lines[g_cursor_line]; RIMCHAR_CH(g_data[i]); i++) {
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
        g_data = realloc(g_data, g_data_max * sizeof(uint32_t));
    }

    // move data after current line
    if (offset) {
        memmove(g_data + g_data_lines[g_cursor_line] + j, g_data + original_len,
                (g_data_count - original_len) * sizeof(uint32_t));
    }

    // copy new line to data buffer
    memcpy(g_data + g_data_lines[g_cursor_line], new_line, j * sizeof(uint32_t));
    free(new_line);

    // update data lines
    for (int i = g_cursor_line + 1; i < g_lines_count; i++)
        g_data_lines[i] += offset;

    g_data_count += offset;
}

void finish_edits(void) {
    reallign_tabs();
    realloc_buffer();
    color_lines(g_cursor_line, g_cursor_line + 1);
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
    } else for (int j = g_data_lines[g_cursor_line]; RIMCHAR_CH(g_data[j]); j++) {
        if (g_data[j] == '\t')
            goto REAL_TAB;
    }

    int spaces = 4 - (g_cursor_pos % 4);
    // realloc buffer if needed
    if (g_data_count + spaces + 2 >= g_data_max) {
        g_data_max += 1024;
        g_data = realloc(g_data, g_data_max * sizeof(uint32_t));
    }

    // add character to data buffer
    for (int i = g_data_count + spaces - 1; i >= spaces && i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
        g_data[i] = g_data[i - spaces];

    for (int i = 0; i < spaces; i++)
        g_data[g_data_lines[g_cursor_line] + g_cursor_pos + i] = RIMCHAR_SN(' ', RIMCHAR_NEW);

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

    int pos = g_data_lines[g_cursor_line] + g_cursor_pos;

    for (int i = pos; i < g_data_count; i++)
        g_data[i] = g_data[i + 1];

    g_data[pos] = RIMCHAR_SN(g_data[pos], RIMCHAR_NEW);

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

    g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = RIMCHAR_NEW | RIMCHAR_NWL;
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

    color_lines(g_cursor_line - 1, g_cursor_line);

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

        memmove(g_data + start, g_data + end, (g_data_count - end) * sizeof(uint32_t));
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
        if (clip == NULL || size == 0)
            return 0;

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

    return 0;
}

/******************************************
 *                                       *
 *            MAIN INPUT LOOP            *
 *                                       *
******************************************/

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
            continue;
        }

        // check if ctrl is released
        else if (key == 157) {
            ctrl_pressed = 0;
            continue;
        }

        // check if key is ctrl + key
        else if (ctrl_pressed) {
            if (key > KB_RVAL)
                continue;
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
            continue;
        }

        // check if shift is released
        else if (key == 170 || key == 182) {
            shift_pressed = 0;
            continue;
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
        refresh_ticks = syscall_ms_get();
        display_data(y_offset, min(g_lines_count, y_offset + SCREEN_H), x_offset);
        refresh_ticks = syscall_ms_get() - refresh_ticks;
    }
}

/******************************************
 *                                       *
 *        COMMAND LINE INTERFACE         *
 *                                       *
******************************************/

void clear_screen(void) {
    syscall_kprint("\e[2J");
    (void) panda_print_string("\e[2J", 4, 0);
}

void quit(void) {
    free(g_panda_buffer);
    free(g_data_lines);
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
        g_rim.syntax->dslcom = 1;
        g_rim.syntax->mltcom = 1;

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
        g_rim.syntax->scpcom = 1;

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
        g_rim.syntax->dslcom = 1;

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

    g_cursor_line = 0;
    g_cursor_pos = 0;

    clear_screen();

    color_lines(0, g_lines_count);

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
