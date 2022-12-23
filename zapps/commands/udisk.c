#include <syscall.h>
#include <stdio.h>

int main(int argc, char **argv) {
    printf("$4total sector count: $1%d\n", c_fs_get_sector_count());
    printf("$4used sector count:  $1%d\n", c_fs_get_used_sectors());
    return 0;
}
