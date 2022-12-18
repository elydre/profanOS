#include <syscall.h>
#include <iolib.h>

int main(int argc, char **argv) {
    fsprint("$4total sector count: $1%d\n", c_fs_get_sector_count());
    fsprint("$4used sector count:  $1%d\n", c_fs_get_used_sectors());
    return 0;
}
