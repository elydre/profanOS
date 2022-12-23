#include <syscall.h>
#include <string.h>
#include <stdlib.h>

int main(int argc, char **argv) {
    char *suffix = malloc(256);
    strcpy(suffix, argv[2]);
    c_task_switch((strcmp(suffix, "yield") == 0) ? 1 : atoi(suffix));
    free(suffix);
    return 0;
}
