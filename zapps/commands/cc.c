#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>
#include <profan.h>


#define TCC_PATH "/bin/fatpath/tcc.bin"     // c compiler
#define VLK_PATH "/bin/fatpath/vlink.bin"   // linker
#define XEC_PATH "/bin/commands/xec.bin"    // elf to binary
#define ZEC_PATH "/sys/zentry.c"            // zentry source
#define ZEO_PATH "/sys/zentry.o"            // zentry object
#define TLC_PATH "/sys/tcclib.c"            // tcclib source
#define TLO_PATH "/sys/tcclib.o"            // tcclib object

#define OPTION_ERROR    1 << 0
#define OPTION_HELP     1 << 1
#define OPTION_RUN      1 << 2
#define OPTION_NOADDS   1 << 3
#define OPTION_NORAND   1 << 4
#define OPTION_CHDEP    1 << 5

uint32_t g_rand_val;

int does_file_exist(char *file) {
    sid_t sid = fu_path_to_sid(ROOT_SID, file);
    return !IS_NULL_SID(sid) && fu_is_file(sid);
}

void new_rand_name(char *name) {
    do {
        for (int i = 0; i < 8; i++) {
            g_rand_val = (g_rand_val * 1103515245 + 12345) & 0x7fffffff;
            name[i] = 'a' + (g_rand_val % 26);
        }
        name[8] = '\0';
    } while (does_file_exist(name));
}

int execute_command(char *path, char *args) {
    printf("\e[37m%s %s\e[0m\n", path, args ? args : "");
    // generate argv

    int argc = 1;
    char **argv;
    if (args == NULL) {
        argv = malloc(2 * sizeof(char *));
    } else {
        argc++;

        for (int i = 0; args[i] != '\0'; i++) {
            if (args[i] == ' ') argc++;
        }

        argv = malloc((argc + 1) * sizeof(char *));

        for (int i = 1; i < argc; i++) {
            argv[i] = args;
            args = strchr(args, ' ');
            *args = '\0';
            args++;
        }
    }

    argv[0] = path;
    argv[argc] = NULL;

    int ret = c_run_ifexist(path, argc, argv);

    free(argv);
    return ret;
}

uint32_t parse_options(int argc, char **argv, char **path) {
    // uint32_t options = parse_options(argc, argv);
    // options & OPTION_HELP;    // -h
    // options & OPTION_RUN;     // -r
    // options & OPTION_NOADDS;  // -n
    // options & OPTION_NORAND;  // -x

    uint32_t options = 0;
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'h':
                        options |= OPTION_HELP;
                        break;
                    case 'r':
                        options |= OPTION_RUN;
                        break;
                    case 'n':
                        options |= OPTION_NOADDS;
                        break;
                    case 'x':
                        options |= OPTION_NORAND;
                        break;
                    case 'c':
                        options |= OPTION_CHDEP;
                        break;
                    default:
                        options |= OPTION_ERROR;
                        printf("cc: invalid option -- '%c'\n", argv[i][j]);
                        break;
                }
            }
        } else {
            *path = argv[i];
        }
    }

    return options;
}


int main(int argc, char **argv) {
    if (argc < 2) {
        printf("Usage: cc [option] <file>\n");
        return 1;
    }

    g_rand_val = c_timer_get_ms();

    char *path = NULL;
    uint32_t options = parse_options(argc, argv, &path);

    char *pwd = getenv("PWD");
    if (!pwd) pwd = "/";

    if (options & OPTION_ERROR) {
        printf("Usage: cc [option] <file>\n");
        return 1;
    }

    if (options & OPTION_CHDEP) {
        printf("Dependencies:\n"
            " tcc.bin     %s\e[0m\n"
            " vlink.bin   %s\e[0m\n"
            " xec.bin     %s\e[0m\n"
            " zentry      %s\e[0m\n"
            " tcclib      %s\e[0m\n",
            does_file_exist(TCC_PATH) ? "\e[92mfound" : "\e[91mnot found",
            does_file_exist(VLK_PATH) ? "\e[92mfound" : "\e[91mnot found",
            does_file_exist(XEC_PATH) ? "\e[92mfound" : "\e[91mnot found",
            does_file_exist(ZEO_PATH) ? "\e[92mfound" : does_file_exist(ZEC_PATH) ? "\e[96msource" : "\e[91mnot found",
            does_file_exist(TLO_PATH) ? "\e[92mfound" : does_file_exist(TLC_PATH) ? "\e[96msource" : "\e[91mnot found"
        );
        return 0;
    }

    if (options & OPTION_HELP) {
        printf("Usage: cc [option] <file>\n");
        printf("Options:\n");
        printf("  -c  Check dependencies\n");
        printf("  -h  Display this information\n");
        printf("  -n  Not include zentry.o, tcclib.o\n");
        printf("  -r  Compile and run\n");
        printf("  -x  Do not use tmp files\n");
        return 0;
    }

    if (path == NULL) {
        printf("cc: no input files\n");
        return 1;
    }

    char *full_path = assemble_path(pwd, path);

    if (!does_file_exist(full_path)) {
        printf("cc: %s file not found\n", full_path);
    } else if (!does_file_exist(TCC_PATH)) {
        printf("cc: tcc.bin not found\n");
    } else if (!does_file_exist(VLK_PATH)) {
        printf("cc: vlink.bin not found\n");
    } else if (!does_file_exist(XEC_PATH)) {
        printf("cc: xec.bin not found\n");
    } else {
        if (!(options & OPTION_NOADDS)) {
            if (!does_file_exist(ZEO_PATH)) {
                if (does_file_exist(ZEC_PATH)) {
                    if (execute_command(TCC_PATH, "-c " ZEC_PATH " -o " ZEO_PATH)) {
                        free(full_path);
                        return 1;
                    }
                } else {
                    printf("cc: zentry.c and zentry.o not found, cannot compile\n");
                    free(full_path);
                    return 1;
                }
            }
            if (!does_file_exist(TLO_PATH)) {
                if (does_file_exist(TLC_PATH)) {
                    if (execute_command(TCC_PATH, "-c " TLC_PATH " -o " TLO_PATH)) {
                        free(full_path);
                        return 1;
                    }
                } else {
                    printf("cc: tcclib.c and tcclib.o not found, cannot compile\n");
                    free(full_path);
                    return 1;
                }
            }
        }

        char *obj_file, *elf_file, *bin_file;
        int ret = 1;

        if (options & OPTION_NORAND) {
            obj_file = malloc(strlen(full_path) + 1);
            strcpy(obj_file, full_path);

            // remove extension from file name
            char *dot = strrchr(obj_file, '.');
            if (dot != NULL) *dot = '\0';

            elf_file = malloc(strlen(obj_file) + 5);
            strcpy(elf_file, obj_file);

            strcat(obj_file, ".o");
            strcat(elf_file, ".elf");
        } else {
            obj_file = malloc(18);
            elf_file = malloc(18);

            strcpy(obj_file, "/tmp/");
            strcpy(elf_file, "/tmp/");

            new_rand_name(obj_file + 5);
            new_rand_name(elf_file + 5);

            strcat(obj_file, ".o");
            strcat(elf_file, ".elf");
        }

        // get the name of the input file
        int len = strlen(full_path);
        for (int i = len - 1; i >= 0; i--) {
            if (full_path[i] == '/') {
                bin_file = assemble_path(pwd, full_path + i + 1);
                break;
            }
        }

        // remove extension from file name
        char *dot = strrchr(bin_file, '.');
        if (dot != NULL) *dot = '\0';

        strcat(bin_file, ".bin");

        char *args = malloc(strlen(full_path) * 2 + 256);

        sprintf(args, "-c %s -o %s", full_path, obj_file);
        if (!execute_command(TCC_PATH, args)) {
            // add zentry.o and tcclib.o to link if -n option is not set
            if (options & OPTION_NOADDS) {
                sprintf(args, "-nostdlib -T /sys/zlink.ld -o %s %s", elf_file, obj_file);
            } else {
                sprintf(args, "-nostdlib -T /sys/zlink.ld -o %s %s %s %s", elf_file, ZEO_PATH, obj_file, TLO_PATH);
            }
            if (!execute_command(VLK_PATH, args)) {
                sprintf(args, "%s %s", elf_file, bin_file);
                if (!execute_command(XEC_PATH, args)) {
                    if (options & OPTION_RUN) {
                        if (!execute_command(bin_file, NULL)) ret = 0;
                    } else ret = 0;
                }
            }
        }

        free(full_path);
        free(elf_file);
        free(obj_file);
        free(bin_file);
        free(args);

        return ret;
    }

    free(full_path);
    return 1;
}
