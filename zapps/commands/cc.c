#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include <syscall.h>
#include <profan.h>


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

int does_file_exist(char *file) {
    // check if path exists
    if (!c_fs_does_path_exists(file))
        return 0;

    // check if path is a file
    if (c_fs_get_sector_type(c_fs_path_to_id(file)) != 2)
        return 0;

    return 1;
}

#define TCC_PATH   "/bin/fatpath/tcc.bin"
#define VLK_PATH   "/bin/fatpath/vlink.bin"
#define XEC_PATH   "/bin/commands/xec.bin"

int main(int argc, char *argv[]) {
    if (argc != 3) {
        printf("Usage: cc <file>\n");
        return 1;
    }

    char *full_path = malloc(strlen(argv[1]) + strlen(argv[2]) + 2);
    assemble_path(argv[1], argv[2], full_path);

    if (!does_file_exist(full_path)) {
        printf("%s file not found\n", full_path);
    } else if (!does_file_exist(TCC_PATH)) {
        printf("tcc.bin not found\n");
    } else if (!does_file_exist(VLK_PATH)) {
        printf("vlink.bin not found\n");
    } else if (!does_file_exist(XEC_PATH)) {
        printf("xec.bin not found\n");
    } else {
        printf("all files found, compiling %s\n", full_path);
        // remove extension from file name
        char *obj_file = malloc(strlen(full_path) + 1);
        strcpy(obj_file, full_path);
        char *dot = strrchr(obj_file, '.');
        if (dot != NULL) *dot = '\0';

        char *elf_file = malloc(strlen(obj_file) + 5);
        strcpy(elf_file, obj_file);

        strcat(obj_file, ".o");
        strcat(elf_file, ".elf");

        char *args = malloc(strlen(full_path) * 2 + 70);
        sprintf(args, "-c %s -o %s", full_path, obj_file);
        execute_command(TCC_PATH, args);

        sprintf(args, "-nostdlib -T /user/zlink.ld %s -o %s", obj_file, elf_file);
        execute_command(VLK_PATH, args);

        execute_command(XEC_PATH, elf_file);

        free(args);
        free(elf_file);
        free(obj_file);
    }
    
    free(full_path);
    return 0;
}
