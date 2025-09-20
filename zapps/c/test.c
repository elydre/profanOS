/*****************************************************************************\
|   === test.c : 2025 ===                                                     |
|                                                                             |
|    -                                                             .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <modules/ahci.h>
#include <stdlib.h>
#include <profan/syscall.h>
#include <stdio.h>
#include <string.h>

#define COUNT 1

int main(void) {
    if (!drive_exists(0)) {
        printf("Drive 0 does not exist!\n");
        return 1;
    }

    printf("Drive 0 exists!\n");

    void *buffer = calloc(512, COUNT);

    printf("read: %d (0 = OK)\n", ahci_read_sectors(0, 0, COUNT, buffer));

    /* for (int i = 0; i < 512 * COUNT; i++) {
        putchar(((uint8_t *)buffer)[i]);
    }*/

    for (int i = 0; i < 512 * COUNT; i++) {
        if (i % 16 == 0) {
            printf("\n%08x: ", i);
        }
        printf("%02x ", ((uint8_t *) buffer)[i]);
    }

    printf("\n");

    free(buffer);
    return 0;
}
