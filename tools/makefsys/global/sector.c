#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "../butterfly.h"

int fs_sector_get_unused(int disk, int count, sid_t *ret) {
    static int next_free_sector = 1;
    *ret = SID_FORMAT(disk, next_free_sector);
    next_free_sector += count;
    return count;
}

int fs_sector_note_free(sid_t sid) {
    printf("not implemented yet\n");
    return 0;
}
