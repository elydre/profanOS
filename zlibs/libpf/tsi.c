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
#include <modules/panda.h>
#include <profan.h>

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

#define TSI_VERSION "0.7.5"

#define TSI_TEXT_FG   0xDDDDDD
#define TSI_TITLE_FG  0x000000
#define TSI_TITLE_BG  0x888888
#define TSI_EOF_FG    0x444444

#define TSI_TAB_SIZE 4

#undef max
#define max(a, b) ((a) > (b) ? (a) : (b))

#undef min
#define min(a, b) ((a) < (b) ? (a) : (b))

int SCREEN_W;
int SCREEN_H;

panda_char_t *tsi_screen_buffer;
void *tsi_old_screen = NULL;

#define tsi_putchar(x, y, c, fg, bg) tsi_screen_buffer[(y) * SCREEN_W + (x)] = \
            ((panda_char_t){.fg_color = fg, .bg_color = bg, .decoration = 0, .character = c})

static void tsi_draw_title(const char *title) {
    int len = strlen(title);
    int i, x = (SCREEN_W - len) / 2;

    for (i = 0; i < x; i++)
        tsi_putchar(i, 0, ' ', TSI_TITLE_FG, TSI_TITLE_BG);
    for (; i < SCREEN_W && title[i - x]; i++)
        tsi_putchar(i, 0, title[i - x], TSI_TITLE_FG, TSI_TITLE_BG);
    for (; i < SCREEN_W; i++)
        tsi_putchar(i, 0, ' ', TSI_TITLE_FG, TSI_TITLE_BG);
}

static void tsi_draw_footer(char *buffer, int line, int line_count) {
    int i;

    snprintf (
            buffer, SCREEN_W,
            " libtsi v"TSI_VERSION"  [%d / %d]  press q to quit, arrows or P/M to scroll",
            line, line_count
    );

    for (i = 0; i < SCREEN_W && buffer[i]; i++)
        tsi_putchar(i, SCREEN_H - 1, buffer[i], TSI_TITLE_FG, TSI_TITLE_BG);
    for (; i < SCREEN_W; i++)
        tsi_putchar(i, SCREEN_H - 1, ' ', TSI_TITLE_FG, TSI_TITLE_BG);
}

static const char **tsi_gen_lines(const char *data, int *line_count_ptr, uint32_t flags) {
    const char **lines = malloc(sizeof(char *) * 2);
    int x, line_count, alloc_size = 2;

    lines[0] = data;
    line_count = 1;
    x = 0;

    for (int i = 0; data[i]; i++) {
        if (data[i] == '\t')
            x += TSI_TAB_SIZE - (x % TSI_TAB_SIZE);
        else
            x++;

        if (data[i] == '\n') {
            lines[line_count] = data + i + 1;
            x = 0;
            line_count++;
        } else if (x >= SCREEN_W && !(flags & TSI_NO_AUTO_WRAP)) {
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
    int x, y;

    for (y = 0; lines[y] && y < SCREEN_H - 2; y++) {
        x = 0;
        for (int i = 0; x < SCREEN_W; i++) {
            if (lines[y][i] != '\n' && lines[y][i] != '\0') {
                if (lines[y][i] == '\t') {
                    do {
                        tsi_putchar(x, y + 1, ' ', TSI_TEXT_FG, 0);
                        x++;
                    } while (x % TSI_TAB_SIZE);
                    continue;
                }
                if (lines[y][i] == ' ')
                    tsi_putchar(x, y + 1, ' ', TSI_TEXT_FG, 0);
                else if (isprint(lines[y][i]) || flags & TSI_NON_PRINTABLE)
                    tsi_putchar(x, y + 1, lines[y][i], TSI_TEXT_FG, 0);
                else
                    tsi_putchar(x, y + 1, '?', TSI_EOF_FG, 0);
                x++;
                continue;
            }
            for (; x < SCREEN_W; x++) {
                tsi_putchar(x, y + 1, ' ', TSI_TEXT_FG, 0);
            }
        }
    }

    for (; y < SCREEN_H - 2; y++) {
        tsi_putchar(0, y + 1, '~', TSI_EOF_FG, 0);
        for (x = 1; x < SCREEN_W; x++) {
            tsi_putchar(x, y + 1, ' ', TSI_TEXT_FG, 0);
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
            panda_print_raw(tsi_screen_buffer, SCREEN_W * SCREEN_H);
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

static int tsi_init_screen(const char *title) {
    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);

    if (SCREEN_W * SCREEN_H == 0 || tsi_old_screen != NULL) {
        return 1;
    }

    tsi_old_screen = panda_screen_backup();

    tsi_screen_buffer = malloc(SCREEN_W * SCREEN_H * sizeof(panda_char_t));

    for (int i = 0; i < SCREEN_W; i++) {
        for (int j = 0; j < SCREEN_H; j++) {
            tsi_putchar(i, j, ' ', TSI_TEXT_FG, 0);
        }
    }

    tsi_draw_title(title);

    return 0;
}

static void tsi_fini(void) {
    panda_screen_restore(tsi_old_screen);
    panda_screen_free(tsi_old_screen);
    free(tsi_screen_buffer);

    tsi_old_screen = NULL;
}

int tsi_start(const char *title, const char *string, uint32_t flags) {
    const char **lines;
    int line_count;

    if (tsi_init_screen(title))
        return 1;

    lines = tsi_gen_lines(string, &line_count, flags);
    tsi_main_loop(lines, line_count, flags);
    free(lines);

    tsi_fini();

    return 0;
}

int tsi_start_array(const char *title, const char **lines, uint32_t flags) {
    int line_count;

    if (tsi_init_screen(title))
        return 1;

    for (line_count = 0; lines[line_count]; line_count++);
    tsi_main_loop(lines, line_count, flags);

    tsi_fini();

    return 0;
}
