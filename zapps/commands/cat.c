#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>

void make_printable(char *str, int size) {
    int i;
    for (i = 0; i < size; i++) {
        if (str[i] == '\0')
            str[i] = 176;
    }
    if (str[size - 1] == '\n') {
        str[size] = '\0';
        return;
    }
    str[size] = '\n';
    str[size + 1] = '\0';
}

void show_help(void) {
    puts("Usage: cat [options] <path>\n"
        "Options:\n"
        "  -h       display this help message\n"
        "  -r       read file in raw mode\n"
        "  -f       read FCTF content\n"
        "  -c -C    canonical display\n"
        "  -s       max file print size\n"
        "Without options, cat will print the file\n"
        "content in printable mode."
    );
}

typedef struct {
    int raw_mode;
    int canonical;
    int max_size;
    int fctf;
    int help;
    char *path;
} cat_args_t;

cat_args_t *parse_args(int argc, char **argv) {
    cat_args_t *args = malloc(sizeof(cat_args_t));
    args->raw_mode = 0;
    args->canonical = 0;
    args->max_size = 0;
    args->help = 0;
    args->path = NULL;

    int i;
    for (i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            if (argv[i][1] == 'h') {
                args->help = 1;
            } else if (argv[i][1] == 'r') {
                args->raw_mode = 1;
            } else if (argv[i][1] == 'f') {
                args->fctf = 1;
            } else if (argv[i][1] == 'c' || argv[i][1] == 'C') {
                args->canonical = 1;
            } else if (argv[i][1] == 's') {
                if (i + 1 >= argc) {
                    printf("Error: -s option requires an argument\n");
                    free(args);
                    exit(1);
                }
                args->max_size = atoi(argv[++i]);
            }
        } else {
            args->path = argv[i];
        }
    }

    return args;
}

int main(int argc, char **argv) {
    cat_args_t *args = parse_args(argc, argv);
    if (args->help) {
        show_help();
        free(args);
        return 0;
    }

    if (!args->path) {
        printf("Error: no path specified\n");
        free(args);
        return 1;
    }

    if (args->raw_mode && args->fctf) {
        printf("Error: -r and -f options are incompatible\n");
        free(args);
        return 1;
    }

    if (args->max_size < 0) {
        printf("Error: -s option requires a positive integer\n");
        free(args);
        return 1;
    }

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    char *path = malloc(strlen(pwd) + strlen(args->path) + 2);
    assemble_path(pwd, args->path, path);

    sid_t file = fu_path_to_sid(ROOT_SID, path);

    if (IS_NULL_SID(file)) {
        printf("Error: '%s' unreachable path\n", path);
        free(path);
        free(args);
        return 1;
    }

    free(path);

    if (args->raw_mode) {
        int size = c_fs_cnt_get_size(c_fs_get_main(), file);
        if (args->max_size && size > args->max_size)
            size = args->max_size;

        char *char_content = malloc(size + 1);

        c_fs_cnt_read(c_fs_get_main(), file, char_content, 0, size);
        if (args->canonical)
            profan_print_memory(char_content, size);
        else
            fwrite(char_content, 1, size, stdout);

        free(char_content);
        free(args);
        return 0;
    }

    if (args->fctf) {
        if (!args->max_size) {
            printf("Error: -s option is required for FCTF files\n");
            free(args);
            return 1;
        }

        if (!fu_is_fctf(file)) {
            printf("Error: '%s' is not a FCTF file\n", args->path);
            free(args);
            return 1;
        }

        char *char_content = malloc(args->max_size + 1);

        fu_fctf_read(file, char_content, 0, args->max_size);
        if (args->canonical)
            profan_print_memory(char_content, args->max_size);
        else
            fwrite(char_content, 1, args->max_size, stdout);

        free(char_content);
        free(args);
        return 0;
    }

    if (!fu_is_file(file)) {
        printf("Error: '%s' is not a file\n", args->path);
        free(args);
        return 1;
    }

    int size = fu_get_file_size(file);

    if (args->max_size && size > args->max_size)
        size = args->max_size;

    char *char_content = malloc(size + 2);

    fu_file_read(file, char_content, 0, size);
    if (args->canonical)
        profan_print_memory(char_content, size);
    else {
        make_printable(char_content, size);
        fputs(char_content, stdout);
    }

    free(char_content);
    free(args);
    return 0;
}
