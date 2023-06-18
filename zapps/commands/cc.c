#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>


#define TCC_PATH "/bin/fatpath/tcc.bin"     // c compiler
#define VLK_PATH "/bin/fatpath/vlink.bin"   // linker
#define XEC_PATH "/bin/commands/xec.bin"    // elf to binary
#define ZEC_PATH "/sys/zentry.c"            // zentry source
#define ZEO_PATH "/sys/zentry.o"            // zentry object

#define OPTION_HELP     1 << 0
#define OPTION_RUN      1 << 1
#define OPTION_NOENTRY  1 << 2


int does_file_exist(char *file) {
    // check if path exists
    if (!c_fs_does_path_exists(file))
        return 0;

    // check if path is a file
    if (c_fs_get_sector_type(c_fs_path_to_id(file)) != 2)
        return 0;

    return 1;
}

void execute_command(char *path, char *args) {
    printf("$6%s %s\n", path, args);
    // generate argv

    int argc = 3;
    for (int i = 0; args[i] != '\0'; i++) {
        if (args[i] == ' ') argc++;
    }

    char **argv = malloc((argc + 1) * sizeof(char *));
    argv[0] = path;
    argv[1] = "/";

    for (int i = 2; i < argc; i++) {
        argv[i] = args;
        args = strchr(args, ' ');
        *args = '\0';
        args++;
    }

    argv[argc] = NULL;

    c_run_ifexist(path, argc, argv);

    free(argv);
}

uint32_t parse_options(int argc, char **argv, char **path) {
    // uint32_t options = parse_options(argc, argv);
    // options & OPTION_HELP;    // -h
    // options & OPTION_RUN;     // -r
    // options & OPTION_NOENTRY; // -n


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
                        options |= OPTION_NOENTRY;
                        break;
                    default:
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


int main(int argc, char *argv[]) {
    if (argc < 3) {
        printf("Usage: cc [option] <file>\n");
        return 1;
    }

    char *path = NULL;
    uint32_t options = parse_options(argc, argv, &path);

    if (options & OPTION_HELP) {
        printf("Usage: cc [option] <file>\n");
        printf("Options:\n");
        printf("  -h  Display this information\n");
        printf("  -n  Do not include zentry.o\n");
        printf("  -r  Compile and run\n");
        return 0;
    }

    if (path == NULL) {
        printf("cc: no input files\n");
        return 1;
    }

    char *full_path = malloc(strlen(argv[1]) + strlen(path) + 2);
    assemble_path(argv[1], path, full_path);

    if (!does_file_exist(full_path)) {
        printf("%s file not found\n", full_path);
    } else if (!does_file_exist(TCC_PATH)) {
        printf("tcc.bin not found\n");
    } else if (!does_file_exist(VLK_PATH)) {
        printf("vlink.bin not found\n");
    } else if (!does_file_exist(XEC_PATH)) {
        printf("xec.bin not found\n");
    } else {
        if (!(options & OPTION_NOENTRY) && !does_file_exist(ZEO_PATH)) {
            if (does_file_exist(ZEC_PATH)) {
                execute_command(TCC_PATH, "-c " ZEC_PATH " -o " ZEO_PATH);
            } else {
                printf("zentry.c and zentry.o not found, cannot compile\n");
                free(full_path);
                return 1;
            }
        }

        // remove extension from file name
        char *obj_file = malloc(strlen(full_path) + 1);
        strcpy(obj_file, full_path);
        char *dot = strrchr(obj_file, '.');
        if (dot != NULL) *dot = '\0';

        char *elf_file = malloc(strlen(obj_file) + 5);
        strcpy(elf_file, obj_file);

        char *bin_file = malloc(strlen(obj_file) + 5);
        strcpy(bin_file, obj_file);

        strcat(obj_file, ".o");
        strcat(elf_file, ".elf");
        strcat(bin_file, ".bin");

        char *args = malloc(strlen(full_path) * 2 + 70);
        sprintf(args, "-c %s -o %s", full_path, obj_file);
        execute_command(TCC_PATH, args);

        // add zentry.o to link if -n option is not set
        if (options & OPTION_NOENTRY) {
            sprintf(args, "-nostdlib -T /sys/zlink.ld -o %s %s", elf_file, obj_file);
        } else {
            sprintf(args, "-nostdlib -T /sys/zlink.ld -o %s %s %s", elf_file, ZEO_PATH, obj_file);
        }

        execute_command(VLK_PATH, args);

        execute_command(XEC_PATH, elf_file);

        // run binary if -r option is set
        if (options & OPTION_RUN) {
            execute_command(bin_file, "");
        }

        free(elf_file);
        free(obj_file);
        free(bin_file);

        free(args);
    }
    
    free(full_path);
    return 0;
}
