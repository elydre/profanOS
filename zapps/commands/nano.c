#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

int main(int argc, char **argv) {
    printf("argc = %d\n", argc);
    for (int i = 0; i < argc; i++) {
        printf("argv[%d] = %s\n", i, argv[i]);
    }
    if (argc == 2) {
        // we open a new file that will be saved later
    }
    else {
        // we open the file that will be edited
        char *filename = calloc(strlen(argv[1]) + 1 + strlen(argv[2]), sizeof(char));
        strcpy(filename, argv[1]);
        strcat(filename, "/");
        strcat(filename, argv[2]);
        strcat(filename, "\0");
        printf("Opening file %s\n", filename);
        FILE *file = fopen(filename, "r+");
        if (file == NULL) {
            printf("File %s does not exist\n", filename);
            return 1;
        }
        free(filename);
        printf("Editing file %s\n", file->name);
    }
}