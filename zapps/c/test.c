#include <profan/ahci.h>
#include <stdlib.h>
#include <profan/syscall.h>
#include <stdio.h>
#include <string.h>

#define COUNT 3

int main(void) {
    if (!drive_exists(0)) {
        printf("Drive 0 does not exist!\n");
        return 1;
    }

    printf("Drive 0 exists!\n");

    void *buffer = calloc(512, COUNT);

    printf("read: %d (0 = OK)\n", ahci_read_sectors(0, 0, COUNT, buffer));

    for (int i = 0; i < 512 * COUNT; i++) {
        putchar(((uint8_t *)buffer)[i]);
    }

    printf("\n");

    free(buffer);
    return 0;
}
