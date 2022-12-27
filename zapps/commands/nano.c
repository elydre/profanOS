#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <syscall.h>

char **text_buffer = NULL;

int main(int argc, char **argv) {

    text_buffer = malloc(1 * sizeof(char*)); // we will realloc later
    text_buffer[0] = calloc(1, sizeof(char)); // we will realloc later

    if (argc == 2) {
        // we open a new file that will be saved later
    }
    else {
        // we open the file that will be edited
        char *filename = calloc(strlen(argv[1]) + 1 + strlen(argv[2]) + 1, sizeof(char));
        strcpy(filename, argv[1]);
        strcat(filename, "/");
        strcat(filename, argv[2]);
        FILE *file = fopen(filename, "r+");
        if (file == NULL) {
            printf("File %s does not exist\n", filename);
            return 1;
        }
        free(filename);
    }
}