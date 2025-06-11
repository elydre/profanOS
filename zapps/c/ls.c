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
    int format;     // -f, -l, -m
    int sort_mode;  // -z
    int showall;    // -a
    int phys_size;  // -p
    int color;      // -c, -n
    char **paths;
} ls_args_t;

#define LS_COLOR_DIR  "\e[96m"
#define LS_COLOR_FILE "\e[92m"
#define LS_COLOR_SHBG "\e[4m"
#define LS_COLOR_AFFT "\e[93m"
#define LS_COLOR_ERR  "\e[91m"
#define LS_COLOR_RSET "\e[0m"

enum {
    LS_FORMAT_COMMA,
    LS_FORMAT_LINES,
    LS_FORMAT_COLS,
    LS_FORMAT_BASIC
};

enum {
    LS_SORT_ALPHA,
    LS_SORT_SIZE
};

typedef struct {
    uint32_t sid;
    char *name;
} ls_entry_t;

/*******************************
 *                            *
 *  terminal tools functions  *
 *                            *
*******************************/

int get_terminal_width(void) {
    uint32_t width, height;

    char *term = getenv("TERM");
    if (term == NULL || strcmp(term, "/dev/panda"))
        return 80;
    panda_get_size(&width, &height);
    return width;
}

int pretty_size_print(uint32_t n) {
    const char *units[] = {"B", "kB", "MB", "GB", "TB"};
    double value = n;
    int unit_index = 0;

    if (value < 1000) {
        return printf("%7d B", n);
    }

    for (; value >= 1000 && unit_index < 4; unit_index++)
        value /= 1000.0;

    if (value < 10)
        return printf("%7.2f %s", value, units[unit_index]);

    if (value < 100)
        return printf("%7.1f %s", value, units[unit_index]);

    return printf("%7.1f %s", value, units[unit_index]);
}

/******************************
 *                           *
 *  entry sorting functions  *
 *                           *
******************************/

int ls_cmp_alpha(const void *p1, const void *p2) {
    char *s1 = ((ls_entry_t *) p1)->name;
    char *s2 = ((ls_entry_t *) p2)->name;

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

int ls_cmp_size(const void *p1, const void *p2) {
    uint32_t s1 = ((ls_entry_t *) p1)->sid;
    uint32_t s2 = ((ls_entry_t *) p2)->sid;

    if (fu_is_dir(s1) && fu_is_dir(s2))
        return fu_dir_get_size(s2) - fu_dir_get_size(s1);

    return syscall_fs_get_size(NULL, s2) - syscall_fs_get_size(NULL, s1);
}

void sort_entries(int count, ls_entry_t *entries, int (*cmp)(const void *, const void *)) {
    uint32_t tmp_id;
    char *tmp_name;

    if (count < 2)
        return;

    int dir_count = 0;

    // put directories first
    for (int i = 0; i < count; i++) {
        if (!fu_is_dir(entries[i].sid))
            continue;

        tmp_name = entries[dir_count].name;
        entries[dir_count].name = entries[i].name;
        entries[i].name = tmp_name;

        tmp_id = entries[dir_count].sid;
        entries[dir_count].sid = entries[i].sid;
        entries[i].sid = tmp_id;

        dir_count++;
    }

    qsort(entries, dir_count, sizeof(ls_entry_t), cmp);
    qsort(entries + dir_count, count - dir_count, sizeof(ls_entry_t), cmp);
}


/*******************************
 *                            *
 *  entry printing functions  *
 *                            *
*******************************/

int print_name(ls_entry_t *entry, ls_args_t *args) {
    if (args->color == 0)
        return fputs(entry->name, stdout);
    if (fu_is_dir(entry->sid))
        return printf(LS_COLOR_DIR "%s" LS_COLOR_RSET, entry->name);
    if (fu_is_afft(entry->sid))
        return printf(LS_COLOR_AFFT "%s" LS_COLOR_RSET, entry->name);
    if (!fu_is_file(entry->sid))
        return printf(LS_COLOR_ERR "%s" LS_COLOR_RSET, entry->name);

    uint8_t buf[2];

    if (fu_file_get_size(entry->sid) > 1) {
        syscall_fs_read(NULL, entry->sid, buf, 0, 4);
        if ((buf[0] == '#' || buf[0] == '>') && buf[1] == '!')
            fputs(LS_COLOR_SHBG, stdout);
    }

    return printf(LS_COLOR_FILE "%s" LS_COLOR_RSET, entry->name);
}

void print_comma(int elm_count, ls_entry_t *entries, ls_args_t *args) {
    for (int i = 0; i < elm_count; i++) {
        if (i)
            fputs(", ", stdout);
        print_name(entries + i, args);
    }
    putchar('\n');
}

void print_basic(int elm_count, ls_entry_t *entries, ls_args_t *args) {
    for (int i = 0; i < elm_count; i++) {
        print_name(entries + i, args);
        putchar('\n');
    }
}

void print_cols(int elm_count, ls_entry_t *entries, ls_args_t *args) {
    uint32_t max_len, *lens, *col_lens;
    int cols, rows, term_w;
    rows = cols = 0;

    col_lens = malloc(elm_count * sizeof(uint32_t));
    lens = malloc(elm_count * sizeof(uint32_t));

    for (int i = 0; i < elm_count; i++) {
        lens[i] = strlen(entries[i].name);
    }

    term_w = get_terminal_width();

    for (int raw = 1; raw <= elm_count; raw++) {
        int width, k;

        cols = (elm_count + raw - 1) / raw;
        rows = (elm_count + cols - 1) / cols;

        width = k = 0;

        for (int i = 0; i < cols; i++) {
            max_len = 0;
            for (int j = 0; j < rows; j++) {
                if (k >= elm_count)
                    break;
                if (lens[k] > max_len)
                    max_len = lens[k];
                k++;
            }
            max_len += 2;
            col_lens[i] = max_len;
            width += max_len;
        }
        if (width <= term_w)
            break;
    }

    for (int i = 0; i < rows; i++) {
        for (int j = i; j < elm_count; j += rows) {
            print_name(entries + j, args);
            if (j + rows < elm_count && cols > 1)
                for (uint32_t k = lens[j]; k < col_lens[j / rows]; k++)
                    putchar(' ');
        }
        putchar('\n');
    }

    free(col_lens);
    free(lens);
}

void print_lines(int elm_count, ls_entry_t *entries, ls_args_t *args) {
    int len;

    for (int i = 0; i < elm_count; i++) {
        printf("%02x:%06x ", SID_DISK(entries[i].sid), SID_SECTOR(entries[i].sid));

        if (args->phys_size == 1)
            len = printf("%8d B", syscall_fs_get_size(NULL, entries[i].sid));
        else if (fu_is_dir(entries[i].sid))
            len = printf("%7d e", fu_dir_get_size(entries[i].sid));
        else if (fu_is_file(entries[i].sid))
            len = pretty_size_print(syscall_fs_get_size(NULL, entries[i].sid));
        else if (fu_is_afft(entries[i].sid))
            len = printf(" F%08x", (uint32_t) fu_afft_get_addr(entries[i].sid));
        else
            len = printf(" ? unk ?");

        for (int j = len; j < 12; j++)
            putchar(' ');
        print_name(entries + i, args);
        putchar('\n');
    }
}


/******************************
 *                           *
 *  entry listing functions  *
 *                           *
******************************/

void list_entries(ls_entry_t *entries, int elm_count, ls_args_t *args) {
    if (args->sort_mode == LS_SORT_ALPHA) {
        sort_entries(elm_count, entries, ls_cmp_alpha);
    } else {
        sort_entries(elm_count, entries, ls_cmp_size);
    }

    if (args->format == LS_FORMAT_COMMA)
        print_comma(elm_count, entries, args);
    else if (args->format == LS_FORMAT_COLS)
        print_cols(elm_count, entries, args);
    else if (args->format == LS_FORMAT_LINES)
        print_lines(elm_count, entries, args);
    else
        print_basic(elm_count, entries, args);
}

void list_files(ls_args_t *args) {
    ls_entry_t *entries = NULL;
    int entry_count = 0;


    for (int i = 0; args->paths[i]; i++) {
        uint32_t sid = profan_path_resolve(args->paths[i]);

        if (IS_SID_NULL(sid)) {
            fprintf(stderr, "ls: %s: No such file or directory\n", args->paths[i]);
            continue;
        }

        if (fu_is_dir(sid)) {
            continue;
        }

        entries = realloc(entries, (entry_count + 1) * sizeof(ls_entry_t));
        entries[entry_count].name = args->paths[i];
        entries[entry_count].sid = sid;
        entry_count++;
    }

    if (entries == NULL)
        return;

    list_entries(entries, entry_count, args);
    free(entries);
}

void list_dirs(ls_args_t *args) {
    int entry_count;

    uint32_t sid;
    char *name;

    for (int i = 0; args->paths[i]; i++) {
        sid = profan_path_resolve(args->paths[i]);

        if (!fu_is_dir(sid))
            continue; // error message is printed by list_files

        uint32_t size = syscall_fs_get_size(NULL, sid);
        if (size == UINT32_MAX || size < sizeof(uint32_t))
            continue;

        uint8_t *buf = malloc(size);
        if (syscall_fs_read(NULL, sid, buf, 0, size)) {
            free(buf);
            continue;
        }

        int elm_count = *(uint32_t *) buf;

        ls_entry_t *entries = calloc(elm_count, sizeof(ls_entry_t));

        entry_count = 0;
        for (int j = 0; j < elm_count; j++) {
            int offset = fu_dir_get_elm(buf, size, j, &sid);

            if (offset <= 0) {
                fprintf(stderr, "ls: error reading directory (%s)\n", strerror(-offset));
                break;
            }

            name = (char *) buf + offset;

            if (!args->showall && *name == '.' &&
                    (name[1] == '\0' || (name[1] == '.' && name[2] == '\0')))
                continue;

            entries[entry_count].name = name;
            entries[entry_count].sid = sid;

            entry_count++;
        }

        if (args->paths[1] != NULL) {
            printf("\n%s:\n", args->paths[i]);
        }

        list_entries(entries, entry_count, args);

        free(entries);
        free(buf);
    }
}

/*********************************
 *                              *
 *  argument parsing functions  *
 *                              *
*********************************/

#define LS_USAGE "Usage: ls [options] [path1] [path2] ...\n"
#define LS_INFO "Try 'ls -h' for more information.\n"

int print_help(void) {
    puts(
        LS_USAGE
        "\nDisplay Options:\n"
        "  -f    line by line display\n"
        "  -l    detailed list view\n"
        "  -m    comma separated list\n"
        "\nSorting Options:\n"
        "  -a    show all elements\n"
        "  -p    use physical size\n"
        "  -z    sort by size\n"
        "\nMisc. Options:\n"
        "  -h    display this help\n"
        "  -c    force color output (ansi escape)\n"
        "  -n    force no color output"
    );
    return 0;
}

ls_args_t *parse_args(int argc, char **argv) {
    ls_args_t *args = calloc(1, sizeof(ls_args_t) + (argc + 1) * sizeof(char *));
    args->format = isatty(1) ? LS_FORMAT_COLS : LS_FORMAT_BASIC;
    args->color = args->format == LS_FORMAT_COLS;

    args->sort_mode = LS_SORT_ALPHA;

    args->paths = (char **) (args + 1);
    int path_count = 0;

    for (int i = 1; i < argc; i++) {
        if (argv[i][0] != '-') {
            args->paths[path_count++] = argv[i];
            continue;
        }

        if (argv[i][1] == '\0') {
            fputs(LS_USAGE LS_INFO, stderr);
            free(args);
            exit(1);
        }

        if (argv[i][1] == '-' && argv[i][2] == '\0') {
            while (++i < argc)
                args->paths[path_count++] = argv[i];
            break;
        }

        for (int j = 1; argv[i][j]; j++) {
            switch (argv[i][j]) {
                case 'f':
                    args->format = LS_FORMAT_BASIC;
                    break;
                case 'l':
                    args->format = LS_FORMAT_LINES;
                    break;
                case 'm':
                    args->format = LS_FORMAT_COMMA;
                    break;
                case 'a':
                    args->showall = 1;
                    break;
                case 'p':
                    args->phys_size = 1;
                    break;
                case 'z':
                    args->sort_mode = LS_SORT_SIZE;
                    break;
                case 'c':
                    args->color = 1;
                    break;
                case 'n':
                    args->color = 0;
                    break;
                case 'h':
                    print_help();
                    free(args);
                    exit(0);
                case '-':
                    fprintf(stderr, "ls: unrecognized option '%s'\n"LS_INFO, argv[i]);
                    free(args);
                    exit(1);
                default:
                    fprintf(stderr, "ls: invalid option -- '%c'\n"LS_INFO, argv[i][j]);
                    free(args);
                    exit(1);
            }
        }
    }

    return args;
}

int main(int argc, char **argv) {
    ls_args_t *args = parse_args(argc, argv);

    if (args->paths[0] == NULL)
        args->paths[0] = ".";

    list_files(args);
    list_dirs(args);

    free(args);
    return 0;
}
