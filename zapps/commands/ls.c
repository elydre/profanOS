#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
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

void sort_alpha_and_type(int count, char **names, sid_t *ids) {
    char *tmp_name;
    sid_t tmp_id;
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
            if (cmp_string_alpha(names[i], names[j]) > 0) {
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
            if (cmp_string_alpha(names[i], names[j]) > 0) {
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

void sort_size_and_type(int count, char **names, sid_t *ids) {
    char *tmp_name;
    sid_t tmp_id;
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
            if (fu_get_file_size(ids[i]) < fu_get_file_size(ids[j])) {
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


int print_help(int full) {
    puts("Usage: ls [options] [path]");

    if (!full) {
        puts("try 'ls -h' for more information");
        return 1;
    }

    puts(
        "Options:\n"
        "  -a  show all elements\n"
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
    args->format = LS_FORMAT_COLS;
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
                    default:
                        printf("ls: invalid option -- '%c'\n", argv[i][j]);
                        args->showhelp = 1;
                        return args;
                }
            }
        } else {
            if (i != argc - 1) {
                printf("ls: invalid argument -- '%s'\n", argv[i]);
                args->showhelp = 1;
                break;
            }
            args->path = argv[i];
        }
    }

    return args;
}

int main(int argc, char **argv) {
    ls_args_t *args = parse_args(argc, argv);
    if (args->showhelp) {
        int ret = print_help(args->showhelp == LS_FULL_HELP);
        free(args);
        return ret;
    }

    char *ls_path;
    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    if (args->path)
        ls_path = assemble_path(pwd, args->path);
    else
        ls_path = strdup(pwd);

    fu_simplify_path(ls_path);

    sid_t dir = fu_path_to_sid(ROOT_SID, ls_path);

    if (IS_NULL_SID(dir) || !fu_is_dir(dir)) {
        printf("%s is not a directory\n", ls_path);
        free(ls_path);
        free(args);
        return 1;
    }

    sid_t *cnt_ids;
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
                free(cnt_names[i]);
                cnt_names[i] = cnt_names[--elm_count];
                cnt_ids[i] = cnt_ids[elm_count];
            } else i++;
        }
    }

    if (!elm_count) {
        free(cnt_names);
        free(cnt_ids);
        free(ls_path);
        free(args);
        return 0;
    }

    if (args->sort_mode == LS_SORT_ALPHA) {
        sort_alpha_and_type(elm_count, cnt_names, cnt_ids);
    } else {
        sort_size_and_type(elm_count, cnt_names, cnt_ids);
    }

    if (args->format == LS_FORMAT_COMMA) {
        for (int i = 0; i < elm_count; i++) {
            if (fu_is_dir(cnt_ids[i])) printf("\e[96m%s", cnt_names[i]);
            else if (fu_is_file(cnt_ids[i])) printf("\e[92m%s", cnt_names[i]);
            else if (fu_is_fctf(cnt_ids[i])) printf("\e[93m%s", cnt_names[i]);
            else printf("\e[91m%s", cnt_names[i]);
            if (i != elm_count - 1) printf("\e[0m, ");
            free(cnt_names[i]);
        }
        puts("\e[0m");
    }

    else if (args->format == LS_FORMAT_COLS) {
        uint32_t max_len = 0;
        for (int i = 0; i < elm_count; i++) {
            if (strlen(cnt_names[i]) > max_len) max_len = strlen(cnt_names[i]);
        }
        max_len++;

        int cols = 80 / (max_len + 1);
        int rows = elm_count / cols + (elm_count % cols ? 1 : 0);

        for (int i = 0; i < rows; i++) {
            for (int j = i; j < elm_count; j += rows) {
                if (fu_is_dir(cnt_ids[j])) printf("\e[96m%s\e[0m", cnt_names[j]);
                else if (fu_is_file(cnt_ids[j])) printf("\e[92m%s\e[0m", cnt_names[j]);
                else if (fu_is_fctf(cnt_ids[j])) printf("\e[93m%s\e[0m", cnt_names[j]);
                else printf("\e[91m%s\e[0m", cnt_names[j]);
                for (uint32_t k = 0; k < max_len - strlen(cnt_names[j]) + 1; k++) putchar(' ');
                free(cnt_names[j]);
            }
            putchar('\n');
        }
    }

    else {
        int size;
        for (int i = 0; i < elm_count; i++) {
            printf("\e[sd%ds%d\e[u\e[10C", cnt_ids[i].device, cnt_ids[i].sector);

            if (fu_is_dir(cnt_ids[i])) {
                if (args->size_type == LS_SIZE_VIRT) {
                    printf("%d elm", fu_get_dir_content(cnt_ids[i], NULL, NULL));
                } else {
                    printf("%d B", c_fs_cnt_get_size(c_fs_get_main(), cnt_ids[i]));
                }
                printf("\e[u\e[22C\e[96m%s\e[0m", cnt_names[i]);
            } else if (fu_is_file(cnt_ids[i])) {
                size = fu_get_file_size(cnt_ids[i]);
                if (args->size_type == LS_SIZE_PHYS || size < 10000) printf("%d B", size);
                else if (size < 10000000) printf("%d kB", size / 1024);
                else printf("%d MB", size / (1024 * 1024));
                printf("\e[u\e[22C\e[92m%s\e[0m", cnt_names[i]);
            } else if (fu_is_fctf(cnt_ids[i])) {
                if (args->size_type == LS_SIZE_VIRT) printf("F:%x", (uint32_t) fu_fctf_get_addr(cnt_ids[i]));
                else printf("%d B", c_fs_cnt_get_size(c_fs_get_main(), cnt_ids[i]));
                printf("\e[u\e[22C\e[93m%s\e[0m", cnt_names[i]);
            } else {
                printf("\e[u\e[22C\e[91m%s\e[0munk", cnt_names[i]);
            }
            free(cnt_names[i]);
            putchar('\n');
        }
    }

    free(cnt_names);
    free(cnt_ids);
    free(ls_path);
    free(args);
    return 0;
}
