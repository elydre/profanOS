#include <syscall.h>
#include <stdio.h>

int main(void) {
    printf("total sector count: %d\n", c_fs_get_sector_count());
    printf("used sector count:  %d\n", c_fs_get_used_sectors());
    return 0;
}
