#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/panda.h>
#include <profan.h>

#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <ctype.h>

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

#define COLOR_T 0x70    // title
#define COLOR_L 0x87    // line number
#define COLOR_D 0x0F    // data
#define COLOR_M 0x70    // more data (>)
#define COLOR_U 0x80    // unknown character
#define COLOR_W 0x08    // whitespace

// GLOBALS

char *g_data;
int g_data_size;

int *g_data_lines;
int g_lines_count;

int g_cursor_line;
int g_cursor_pos;

int g_always_tab;

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

int word_isnumber(char *word, int size) {
    if (size == 0) return 0;
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
    char *types[] = {"if", "else", "while", "for", "do", "switch", "case", "default", "break", "continue", "return", "goto"};
    for (uint32_t i = 0; i < (sizeof(types) / sizeof(char *)); i++) {
        if (size == strlen(types[i]) && !memcmp(word, types[i], size)) return 1;
    }
    return 0;
}

int word_isblue(char *word, uint32_t size) {
    char *types[] = {"char", "short", "int", "long", "float", "double", "void", "struct", "enum", "union", "signed", "unsigned", "const", "volatile", "static", "extern", "register", "auto", "typedef", "bool", "sizeof", "NULL", "true", "false", "inline", "restrict"};
    for (uint32_t i = 0; i < (sizeof(types) / sizeof(char *)); i++) {
        if (size == strlen(types[i]) && !memcmp(word, types[i], size)) return 1;
    }
    return (size > 2 && word[size - 1] == 't' && word[size - 2] == '_');
}

int word_isbrace(char *word, uint32_t size) {
    return (size == 1 && (word[0] == '(' || word[0] == ')' || word[0] == '{' || word[0] == '}' || word[0] == '[' || word[0] == ']'));
}

int word_paraft(char *word, uint32_t size) {
    for (uint32_t i = size; word[i]; i++) {
        if (word[i] == '(') return 1;
        if (!isspace(word[i])) return 0;
    }
    return 0;
}

void put_word(int in_word, uint16_t *new_screen, int new_screen_i, char *word, int size) {
    char color = 0x0F;
    if (in_word == 2) color = 0x0E;
    else if (in_word == 1) {
        color = 0x07;
        if (word_isnumber(word, size)) color = 0x0A;
        else if (word_isblue(word, size)) color = 0x09;
        else if (word_purple(word, size)) color = 0x0D;
        else if (word_paraft(word, size)) color = 0x06;        
    } else if (word_isbrace(word, size)) color = 0x0B;

    for (int k = 0; k < size; k++) {
        new_screen[new_screen_i + k] = word[k] | (color << 8);
    }
}

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
                put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
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
                put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
                world_start = j + 1;
                if (in_word != 2) in_word = 0;
                continue;
            }
            else if (g_data[g_data_lines[i] + j] == ' ') {
                new_screen[line * SCREEN_W + j - x_offset + x] = '.' | (COLOR_W << 8);
                put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
                world_start = j + 1;
                if (in_word != 2) in_word = 0;
                continue;
            }

            if (in_word == 2 && g_data[g_data_lines[i] + j] == chr_str) {
                put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start + 1);
                world_start = j + 1;
                in_word = 0;

            } else if (g_data[g_data_lines[i] + j] == '\"' || g_data[g_data_lines[i] + j] == '\'') {
                put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
                chr_str = g_data[g_data_lines[i] + j];
                world_start = j;
                in_word = 2;
            }

            else if (in_word != 2) {
                if (isalnum(g_data[g_data_lines[i] + j]) || g_data[g_data_lines[i] + j] == '_') {
                    if (!in_word) {
                        put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
                        world_start = j;
                        in_word = 1;
                    }
                } else {
                    put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, j - world_start);
                    world_start = j;
                    in_word = 0;
                }
            }
        }
        put_word(in_word, new_screen, line * SCREEN_W + x + world_start, g_data + g_data_lines[i] + world_start, max - world_start);
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
    if (g_lines_count % 1024 == 0) {
        g_data_lines = realloc(g_data_lines, (g_lines_count + 1024) * sizeof(int));
    }

    if (g_data_size % 1024 == 0 && g_data_size != 0) {
        g_data = realloc(g_data, (g_data_size + 1024) * sizeof(char));
    }
}

void insert_tab(void) {
    int tab = 0;

    if (g_always_tab) {
        tab = 1;
    } else for (int j = g_data_lines[g_cursor_line]; g_data[j] != '\0'; j++) {
        if (g_data[j] == '\t') {
            tab = 1;
            break;
        }
    }

    if (tab) {
        // add character to data buffer
        for (int i = g_data_size; i > g_data_lines[g_cursor_line] + g_cursor_pos; i--)
            g_data[i] = g_data[i - 1];

        g_data[g_data_lines[g_cursor_line] + g_cursor_pos] = '\t';
        g_data_size++;

        for (int i = g_cursor_line + 1; i < g_lines_count; i++)
            g_data_lines[i]++;

        g_cursor_pos++;
    } else {
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

    clear_screen();
}

char *compute_args(int argc, char **argv) {
    char *file = NULL;
    g_always_tab = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 't') {
                g_always_tab = 1;
            } else if (argv[i][1] == 'h') {
                puts("Usage: rim [-h|-t] [file]"
                    "\nOptions:"
                    "\n  -h    display this help message"
                    "\n  -t    always insert tab character"
                );
                exit(0);
            } else {
                fprintf(stderr, "rim: Unknown option -- '%s'\n", argv[i] + 1);
                exit(1);
            }
        } else {
            file = argv[i];
        }
    }

    if (!file)
        return NULL;

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    file = assemble_path(pwd, file);

    sid_t elm = fu_path_to_sid(ROOT_SID, file);

    if (IS_NULL_SID(elm)) {
        elm = fu_file_create(0, file);
        if (IS_NULL_SID(elm)) {
            fprintf(stderr, "rim: %s: failed to create file\n", file);
            exit(1);
        }
    } else if (!fu_is_file(elm)) {
        fprintf(stderr, "rim :%s: file not found\n", file);
        exit(1);
    }

    return file;
}

int main(int argc, char **argv) {
    char *file = compute_args(argc, argv);

    panda_get_size((uint32_t *) &SCREEN_W, (uint32_t*) &SCREEN_H);
    if (SCREEN_W * SCREEN_H == 0) {
        printf("rim: panda is required\n");
        return 1;
    }
    SCREEN_H--;
    SCREEN_W--;

    if (argc == 2) {
    }

    g_data = calloc(1024, sizeof(char));
    g_data_size = 1;

    g_data_lines = calloc(1024, sizeof(int));
    g_lines_count = 1;

    g_cursor_line = 0;
    g_cursor_pos = 0;

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
