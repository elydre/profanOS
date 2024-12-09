/*****************************************************************************\
|   === tsi.c : 2024 ===                                                      |
|                                                                             |
|    Terminal based scrollable interface library                   .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/libtsi.h>
#include <profan/panda.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define TSI_VERSION "0.7.2"

#define TSI_TEXT_COLOR   0x0F
#define TSI_TITLE_COLOR  0x70
#define TSI_FOOTER_COLOR 0x70
#define TSI_EOF_COLOR    0x08

#define TSI_TAB_SIZE 4

#undef max
#define max(a, b) ((a) > (b) ? (a) : (b))

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

int SCREEN_W;
int SCREEN_H;

static void clear_screen(void) {
    syscall_kprint("\e[2J");
    panda_print_string("\e[2J", 4, -1, 0x0F);
}

static void tsi_draw_title(const char *title) {
    int len = strlen(title);
    int i, x = (SCREEN_W - len) / 2;

    for (i = 0; i < x; i++)
        panda_set_char(i, 0, ' ', TSI_TITLE_COLOR);
    for (; i < SCREEN_W && title[i - x]; i++)
        panda_set_char(i, 0, title[i - x], TSI_TITLE_COLOR);
    for (; i < SCREEN_W; i++)
        panda_set_char(i, 0, ' ', TSI_TITLE_COLOR);
}

static void tsi_draw_footer(char *buffer, int line, int line_count) {
    int i;

    snprintf (
            buffer, SCREEN_W,
            " libtsi v"TSI_VERSION"  [%d / %d]  press q to quit, arrows or P/M to scroll",
            line, line_count
    );

    for (i = 0; i < SCREEN_W && buffer[i]; i++)
        panda_set_char(i, SCREEN_H - 1, buffer[i], TSI_FOOTER_COLOR);
    for (; i < SCREEN_W; i++)
        panda_set_char(i, SCREEN_H - 1, ' ', TSI_FOOTER_COLOR);
}

static uint8_t tsi_ansi_to_panda(char l) {
    switch (l) {
        case '0': return 0;
        case '1': return 4;
        case '2': return 2;
        case '3': return 6;
        case '4': return 1;
        case '5': return 5;
        case '6': return 3;
    }
    return 7;
}

static int tsi_compute_ansi(const char *data, uint8_t *color_ptr) {
    int i, len = 0;

    if (data[0] == '\e' && data[1] == '[') {
        if (data[2] == '0' && data[3] == 'm') {
            if (color_ptr)
                *color_ptr = TSI_TEXT_COLOR;
            return 3;
        }
        if (data[2] == '3' && data[3] >= '0' && data[3] <= '7' && data[4] == 'm') {
            if (color_ptr)
                *color_ptr = tsi_ansi_to_panda(data[3]);
            return 4;
        }
        if (data[2] == '9' && data[3] >= '0' && data[3] <= '7' && data[4] == 'm') {
            if (color_ptr)
                *color_ptr = tsi_ansi_to_panda(data[3]) | 0x08;
            return 4;
        }
    }
    return 0;
}

static const char **tsi_gen_lines(const char *data, int *line_count_ptr) {
    const char **lines = malloc(sizeof(char *) * 2);
    int x, line_count, alloc_size = 2;

    lines[0] = data;
    line_count = 1;
    x = 0;

    for (int i = 0; data[i]; i++) {
        if (data[i] == '\t')
            x += TSI_TAB_SIZE - (x % TSI_TAB_SIZE);
        else if (data[i] == '\e')
            i += tsi_compute_ansi(data + i, NULL);
        else
            x++;

        if (data[i] == '\n') {
            lines[line_count] = data + i + 1;
            x = 0;
            line_count++;
        } else if (x >= SCREEN_W) {
            lines[line_count] = data + i + 1;
            line_count++;
            x = 0;
        }

        if (line_count >= alloc_size - 1) {
            alloc_size *= 2;
            lines = realloc(lines, sizeof(char *) * alloc_size);
        }
    }

    lines[line_count] = NULL;

    *line_count_ptr = line_count;
    return lines;
}

static void tsi_draw_lines(const char **lines, uint32_t flags) {
    uint8_t color = TSI_TEXT_COLOR;
    int x, y;

    for (y = 0; lines[y] && y < SCREEN_H - 2; y++) {
        x = 0;
        for (int i = 0; x < SCREEN_W; i++) {
            if (lines[y][i] != '\n' && lines[y][i] != '\0') {
                if (lines[y][i] == '\t') {
                    do {
                        panda_set_char(x, y + 1, ' ', TSI_TEXT_COLOR);
                        x++;
                    } while (x % TSI_TAB_SIZE);
                    continue;
                }
                if (lines[y][i] == '\e') {
                    i += tsi_compute_ansi(lines[y] + i, &color);
                    continue;
                }
                if (lines[y][i] == ' ')
                    panda_set_char(x, y + 1, ' ', TSI_TEXT_COLOR);
                else if (isprint(lines[y][i]) || flags & TSI_ALLOW_NON_PRINTABLE)
                    panda_set_char(x, y + 1, lines[y][i], color);
                else
                    panda_set_char(x, y + 1, '?', TSI_EOF_COLOR);
                x++;
                continue;
            }
            for (; x < SCREEN_W; x++) {
                panda_set_char(x, y + 1, ' ', TSI_TEXT_COLOR);
            }
        }
    }

    for (; y < SCREEN_H - 2; y++) {
        panda_set_char(0, y + 1, '~', TSI_EOF_COLOR);
        for (x = 1; x < SCREEN_W; x++) {
            panda_set_char(x, y + 1, ' ', TSI_TEXT_COLOR);
        }
    }
}

static void tsi_main_loop(const char **lines, int line_count, uint32_t flags) {
    int key, y = 0;
    char keyc;

    int need_redraw = 1;

    char *buffer = malloc(SCREEN_W + 1);

    do {
        if (need_redraw) {
            tsi_draw_lines(lines + y, flags);
            tsi_draw_footer(buffer, y + 1, line_count);
        }

        key = syscall_sc_get();
        keyc = profan_kb_get_char(key, 0);

        need_redraw = 1;
        if (!line_count);
        else if (key == KB_TOP)
            y = max(y - 1, 0);
        else if (key == KB_BOT)
            y = min(y + 1, line_count - 1);
        else if (keyc == 'm' || keyc == ' ')
            y = min(y + SCREEN_H - 2, line_count - 1);
        else if (keyc == 'p')
            y = max(y - SCREEN_H + 2, 0);
        else
            need_redraw = 0;

        usleep(10000);
    } while (keyc != 'q' && key != KB_ESC);
    free(buffer);
}

int tsi_start(const char *title, const char *string, uint32_t flags) {
    const char **lines;
    int line_count;

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);

    if (SCREEN_W * SCREEN_H == 0) {
        return 1;
    }

    void *old_screen = panda_screen_backup();

    clear_screen();

    tsi_draw_title(title);

    lines = tsi_gen_lines(string, &line_count);

    tsi_main_loop(lines, line_count, flags);

    free(lines);

    panda_screen_restore(old_screen);
    panda_screen_free(old_screen);
    return 0;
}

int tsi_start_array(const char *title, const char **lines, uint32_t flags) {
    int line_count;

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);

    if (SCREEN_W * SCREEN_H == 0) {
        return 1;
    }

    void *old_screen = panda_screen_backup();

    clear_screen();

    tsi_draw_title(title);

    for (line_count = 0; lines[line_count]; line_count++);

    tsi_main_loop(lines, line_count, flags);

    panda_screen_restore(old_screen);
    panda_screen_free(old_screen);
    return 0;
}
