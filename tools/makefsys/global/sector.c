#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../butterfly.h"

int fs_sector_get_unused(int disk_id) {
    static int next_free_sector = 0;
    return next_free_sector++;
}

int fs_sector_note_used(sid_t sid) {
    return 0;
}

int fs_sector_note_free(sid_t sid) {
    printf("not implemented yet\n");
    return 0;
}
