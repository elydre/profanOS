#include <syscall.h>

int main(int argc, char **argv) {
    c_fskprint("$4total sector count: $1%d\n", c_fs_get_sector_count());
    c_fskprint("$4used sector count:  $1%d\n", c_fs_get_used_sectors());
    return 0;
}
