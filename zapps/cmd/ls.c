/*****************************************************************************\
|   === ls.c : 2024 ===                                                       |
|                                                                             |
|    Unix command implementation - list directory contents         .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/panda.h>
#include <profan.h>

typedef struct {
    int format;
    int sort_mode;
    int showall;
    int showhelp;
    int size_type;
    char *path;
} ls_args_t;

#define LS_FORMAT_COMMA 0
#define LS_FORMAT_LINES 1
#define LS_FORMAT_COLS  2
#define LS_FORMAT_BASIC 3

#define LS_SORT_ALPHA   0
#define LS_SORT_SIZE    1

#define LS_NO_HELP      0
#define LS_SHORT_HELP   1
#define LS_FULL_HELP    2

#define LS_SIZE_VIRT    0
#define LS_SIZE_PHYS    1

int cmp_string_alpha(char *s1, char *s2) {
    int i = 0;
    while (s1[i] && s2[i]) {
        if (s1[i] < s2[i]) return -1;
        else if (s1[i] > s2[i]) return 1;
        i++;
    }
    if (s1[i]) return 1;
    else if (s2[i]) return -1;
    else return 0;
}

int get_terminal_width(void) {
    uint32_t width, height;

    char *term = getenv("TERM");
    if (term == NULL || strcmp(term, "/dev/panda"))
        return 80;
    panda_get_size(&width, &height);
    return width;
}

void sort_alpha_and_type(int count, char **names, uint32_t *ids) {
    char *tmp_name;
    uint32_t tmp_id;
    int i, j;

    int dir_count = 0;

    // put directories first
    for (i = 0; i < count; i++) {
        if (fu_is_dir(ids[i])) {
            tmp_name = names[dir_count];
            names[dir_count] = names[i];
            names[i] = tmp_name;

            tmp_id = ids[dir_count];
            ids[dir_count] = ids[i];
            ids[i] = tmp_id;

            dir_count++;
        }
    }

    // sort directories
    for (i = 0; i < dir_count; i++) {
        for (j = i + 1; j < dir_count; j++) {
            if (cmp_string_alpha(names[i], names[j]) <= 0)
                continue;
            tmp_name = names[i];
            names[i] = names[j];
            names[j] = tmp_name;

            tmp_id = ids[i];
            ids[i] = ids[j];
            ids[j] = tmp_id;
        }
    }

    // sort files
    for (i = dir_count; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (cmp_string_alpha(names[i], names[j]) <= 0)
                continue;
            tmp_name = names[i];
            names[i] = names[j];
            names[j] = tmp_name;

            tmp_id = ids[i];
            ids[i] = ids[j];
            ids[j] = tmp_id;
        }
    }
}

void sort_size_and_type(int count, char **names, uint32_t *ids) {
    char *tmp_name;
    uint32_t tmp_id;
    int i, j;

    int dir_count = 0;

    // put directories first
    for (i = 0; i < count; i++) {
        if (fu_is_dir(ids[i])) {
            tmp_name = names[dir_count];
            names[dir_count] = names[i];
            names[i] = tmp_name;

            tmp_id = ids[dir_count];
            ids[dir_count] = ids[i];
            ids[i] = tmp_id;

            dir_count++;
        }
    }

    // sort directories
    for (i = 0; i < dir_count; i++) {
        for (j = i + 1; j < dir_count; j++) {
            if (fu_get_dir_content(ids[i], NULL, NULL) < fu_get_dir_content(ids[j], NULL, NULL)) {
                tmp_name = names[i];
                names[i] = names[j];
                names[j] = tmp_name;

                tmp_id = ids[i];
                ids[i] = ids[j];
                ids[j] = tmp_id;
            }
        }
    }

    // sort files
    for (i = dir_count; i < count; i++) {
        for (j = i + 1; j < count; j++) {
            if (fu_file_get_size(ids[i]) < fu_file_get_size(ids[j])) {
                tmp_name = names[i];
                names[i] = names[j];
                names[j] = tmp_name;

                tmp_id = ids[i];
                ids[i] = ids[j];
                ids[j] = tmp_id;
            }
        }
    }
}


int print_help(void) {
    puts(
        "Usage: ls [options] [path]\n"
        "Options:\n"
        "  -a  show all elements\n"
        "  -f  line by line, no color\n"
        "  -h  display this help\n"
        "  -l  print files in separate lines\n"
        "  -m  print files in comma separated list\n"
        "  -p  show file size in physical size\n"
        "  -z  sort by size"
    );
    return 0;
}

ls_args_t *parse_args(int argc, char **argv) {
    ls_args_t *args = malloc(sizeof(ls_args_t));
    args->format = isatty(1) ? LS_FORMAT_COLS : LS_FORMAT_BASIC;
    args->sort_mode = LS_SORT_ALPHA;
    args->showall = 0;
    args->showhelp = LS_NO_HELP;
    args->size_type = LS_SIZE_VIRT;
    args->path = NULL;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j]; j++) {
                switch (argv[i][j]) {
                    case 'l':
                        args->format = LS_FORMAT_LINES;
                        break;
                    case 'm':
                        args->format = LS_FORMAT_COMMA;
                        break;
                    case 'a':
                        args->showall = 1;
                        break;
                    case 'h':
                        args->showhelp = 2;
                        break;
                    case 'p':
                        args->size_type = LS_SIZE_PHYS;
                        break;
                    case 'z':
                        args->sort_mode = LS_SORT_SIZE;
                        break;
                    case 'f':
                        args->format = LS_FORMAT_BASIC;
                        break;
                    default:
                        fprintf(stderr, "ls: invalid option -- '%c'\n", argv[i][j]);
                        args->showhelp = 1;
                        return args;
                }
            }
        } else {
            if (i != argc - 1) {
                fprintf(stderr, "ls: invalid argument -- '%s'\n", argv[i]);
                args->showhelp = 1;
                break;
            }
            args->path = argv[i];
        }
    }

    return args;
}

void print_name(uint32_t id, char *name) {
    if (fu_is_dir(id)) printf("\e[96m%s", name);
    else if (fu_is_file(id)) printf("\e[92m%s", name);
    else if (fu_is_fctf(id)) printf("\e[93m%s", name);
    else printf("\e[91m%s", name);
}

void print_comma(int elm_count, char **cnt_names, uint32_t *cnt_ids) {
    for (int i = 0; i < elm_count; i++) {
        print_name(cnt_ids[i], cnt_names[i]);
        if (i != elm_count - 1) printf("\e[0m, ");
        profan_kfree(cnt_names[i]);
    }
    puts("\e[0m");
}

void print_cols(int elm_count, char **cnt_names, uint32_t *cnt_ids) {
    uint32_t max_len, *lens, *col_lens;
    int cols, rows, width, k, term_w;
    rows = cols = 0;

    col_lens = malloc(elm_count * sizeof(uint32_t));
    lens = malloc(elm_count * sizeof(uint32_t));

    for (int i = 0; i < elm_count; i++) {
        lens[i] = strlen(cnt_names[i]);
    }

    term_w = get_terminal_width();

    for (int raw = 1; raw <= elm_count; raw++) {
        cols = (elm_count + raw - 1) / raw;
        rows = (elm_count + cols - 1) / cols;
        width = k = 0;

        for (int i = 0; i < cols; i++) {
            max_len = 0;
            for (int j = 0; j < rows; j++) {
                if (k >= elm_count) break;
                if (lens[k] > max_len) max_len = lens[k];
                k++;
            }
            max_len += 2;
            col_lens[i] = max_len;
            width += max_len;
        }
        if (width <= term_w) break;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = i; j < elm_count; j += rows) {
            print_name(cnt_ids[j], cnt_names[j]);
            if (j + rows < elm_count && cols > 1)
                for (uint32_t k = lens[j]; k < col_lens[j / rows]; k++)
                    putchar(' ');
            profan_kfree(cnt_names[j]);
        }
        putchar('\n');
    }

    fputs("\e[0m", stdout);

    free(col_lens);
    free(lens);
}

void print_lines(int elm_count, char **cnt_names, uint32_t *cnt_ids, ls_args_t *args) {
    int size;
    for (int i = 0; i < elm_count; i++) {
        printf("\e[sd%ds%d\e[u\e[10C", SID_DISK(cnt_ids[i]), SID_SECTOR(cnt_ids[i]));

        if (fu_is_dir(cnt_ids[i])) {
            if (args->size_type == LS_SIZE_VIRT) {
                printf("%d elm", fu_get_dir_content(cnt_ids[i], NULL, NULL));
            } else {
                printf("%d B", syscall_fs_get_size(NULL, cnt_ids[i]));
            }
            printf("\e[u\e[22C\e[96m%s\e[0m", cnt_names[i]);
        } else if (fu_is_file(cnt_ids[i])) {
            size = fu_file_get_size(cnt_ids[i]);
            if (args->size_type == LS_SIZE_PHYS || size < 10000) printf("%d B", size);
            else if (size < 10000000) printf("%d kB", size / 1024);
            else printf("%d MB", size / (1024 * 1024));
            printf("\e[u\e[22C\e[92m%s\e[0m", cnt_names[i]);
        } else if (fu_is_fctf(cnt_ids[i])) {
            if (args->size_type == LS_SIZE_VIRT) printf("F:%x", (uint32_t) fu_fctf_get_addr(cnt_ids[i]));
            else printf("%d B", syscall_fs_get_size(NULL, cnt_ids[i]));
            printf("\e[u\e[22C\e[93m%s\e[0m", cnt_names[i]);
        } else {
            printf("unk\e[u\e[22C\e[91m%s\e[0m", cnt_names[i]);
        }
        profan_kfree(cnt_names[i]);
        putchar('\n');
    }
}

void print_basic(int elm_count, char **cnt_names) {
    for (int i = 0; i < elm_count; i++) {
        puts(cnt_names[i]);
        profan_kfree(cnt_names[i]);
    }
}

int main(int argc, char **argv) {
    ls_args_t *args = parse_args(argc, argv);
    if (args->showhelp == LS_SHORT_HELP) {
        fputs("Try 'ls -h' for more information.\n", stderr);
        free(args);
        return 1;
    }

    if (args->showhelp) {
        print_help();
        free(args);
        return 0;
    }

    char *ls_path;
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    if (args->path)
        ls_path = profan_join_path(pwd, args->path);
    else
        ls_path = strdup(pwd);

    fu_simplify_path(ls_path);

    uint32_t dir = fu_path_to_sid(SID_ROOT, ls_path);

    if (IS_SID_NULL(dir) || !fu_is_dir(dir)) {
        fprintf(stderr, "ls: %s: No such directory\n", ls_path);
        free(ls_path);
        free(args);
        return 1;
    }

    uint32_t *cnt_ids;
    char **cnt_names;

    int elm_count = fu_get_dir_content(dir, &cnt_ids, &cnt_names);

    if (!elm_count) {
        free(ls_path);
        free(args);
        return 0;
    }

    if (!args->showall) {
        int i = 0;
        while (i < elm_count) {
            if (cnt_names[i][0] == '.') {
                profan_kfree(cnt_names[i]);
                cnt_names[i] = cnt_names[--elm_count];
                cnt_ids[i] = cnt_ids[elm_count];
            } else i++;
        }
    }

    if (!elm_count) {
        profan_kfree(cnt_names);
        profan_kfree(cnt_ids);
        free(ls_path);
        free(args);
        return 0;
    }

    if (args->sort_mode == LS_SORT_ALPHA) {
        sort_alpha_and_type(elm_count, cnt_names, cnt_ids);
    } else {
        sort_size_and_type(elm_count, cnt_names, cnt_ids);
    }

    if (args->format == LS_FORMAT_COMMA)
        print_comma(elm_count, cnt_names, cnt_ids);
    else if (args->format == LS_FORMAT_COLS)
        print_cols(elm_count, cnt_names, cnt_ids);
    else if (args->format == LS_FORMAT_LINES)
        print_lines(elm_count, cnt_names, cnt_ids, args);
    else
        print_basic(elm_count, cnt_names);


    profan_kfree(cnt_names);
    profan_kfree(cnt_ids);
    free(ls_path);
    free(args);
    return 0;
}
