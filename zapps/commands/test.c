// test /dev/rand

#include <filesys.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>


int main(void) {
    uint8_t *buffer = calloc(100, sizeof(uint8_t));

    /* profanOS filesys.h
    sid_t file = fu_path_to_sid(ROOT_SID, "/dev/random");
    fu_fctf_read(file, buffer, 0, 100);
    */

    // stdio.h
    FILE *file = fopen("/dev/random", "r");
    fread(buffer, sizeof(uint8_t), 100, file);
    fclose(file);

    for (int i = 0; i < 100; i++) {
        printf("%d ", buffer[i]);
    }
    printf("\n");

    free(buffer);

    return 0;
}
