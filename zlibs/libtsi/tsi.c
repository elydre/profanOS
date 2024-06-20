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
#include <stdio.h>
#include <unistd.h>

#define TSI_VERSION "0.1"

int SCREEN_W;
int SCREEN_H;

static void clear_screen(void) {
    c_kprint("\e[2J");
    fputs("\e[2J", stdout);
    fflush(stdout);
}

static void tsi_draw_title(const char *title) {
    int len = strlen(title);
    int i, x = (SCREEN_W - len) / 2;

    for (i = 0; i < x; i++)
        panda_set_char(i, 0, ' ', 0x70);
    for (; i < SCREEN_W && title[i - x]; i++)
        panda_set_char(i, 0, title[i - x], 0x70);
    for (; i < SCREEN_W; i++)
        panda_set_char(i, 0, ' ', 0x70);
}

static void tsi_draw_footer(void) {
    char *footer = "libtsi v"TSI_VERSION" - press q to quit, arrows to scroll, P/M to move page";
    int i;
    for (i = 0; i < SCREEN_W && footer[i]; i++)
        panda_set_char(i, SCREEN_H - 1, footer[i], 0x70);
    for (; i < SCREEN_W; i++)
        panda_set_char(i, SCREEN_H - 1, ' ', 0x70);
}

static void tsi_draw_lines(const char **lines) {
    int x, y;

    for (y = 0; lines[y] && y < SCREEN_H - 2; y++) {
        x = 0;
        for (int i = 0; x < SCREEN_W; i++) {
            if (lines[y][i] != '\n' && lines[y][i] != '\0') {
                panda_set_char(x, y + 1, lines[y][i], 0x07);
                x++;
                continue;
            }
            for (; x < SCREEN_W; x++) {
                panda_set_char(x, y + 1, ' ', 0x07);
            }
        }
    }

    for (; y < SCREEN_H - 2; y++) {
        panda_set_char(0, y + 1, '~', 0x08);
        for (x = 1; x < SCREEN_W; x++) {
            panda_set_char(x, y + 1, ' ', 0x07);
        }
    }
}

static const char **tsi_gen_lines(const char *data, int *line_count_ptr) {
    const char **lines;
    int line_count = 0;

    int i, x = 0;
    for (i = 0; data[i]; i++) {
        x++;
        if (data[i] == '\n') {
            line_count++;
            x = 0;
        } else if (x > SCREEN_W) {
            line_count++;
            x = 0;
        }
    }

    lines = malloc(sizeof(char *) * (line_count + 1));
    serial_debug("line_count: %d\n", line_count);
    lines[line_count] = NULL;

    line_count = 0;
    x = 0;
    for (i = 0; data[i]; i++) {
        if (i == 0) {
            lines[line_count] = data + i;
            line_count++;
        } else if (data[i] == '\n') {
            lines[line_count] = data + i + 1;
            x = 0;
            line_count++;
        } else if (x > SCREEN_W) {
            lines[line_count] = data + i - x + 1;
            line_count++;
            x = 0;
        }
        x++;
    }

    serial_debug("line_count: %d\n", line_count);

    *line_count_ptr = line_count;
    return lines;
}

static void tsi_main_loop(const char **lines, int line_count) {
    int y = 0;
    char keyc;

    int need_redraw = 1;

    do {
        if (need_redraw) {            
            tsi_draw_lines(lines + y);
            tsi_draw_footer();
        }

        int key = c_kb_get_scfh();
        keyc = profan_kb_get_char(key, 0);

        need_redraw = 1;
        if (key == KB_TOP) {
            y--;
            if (y < 0) {
                y = 0;
            }
        } else if (key == KB_BOT) {
            y++;
            if (y >= line_count) {
                y = line_count - 1;
            }
        } else {
            need_redraw = 0;
        }

        usleep(10000);
    } while (keyc != 'q');
}

int tsi_start(const char *title, const char *data) {
    const char **lines;
    int line_count;

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);

    if (SCREEN_W * SCREEN_H == 0) {
        return 1;
    }

    clear_screen();

    tsi_draw_title(title);

    lines = tsi_gen_lines(data, &line_count);

    tsi_main_loop(lines, line_count);

    clear_screen();
    free(lines);

    return 0;
}
