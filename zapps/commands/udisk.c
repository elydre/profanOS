#include <syscall.h>
#include <iolib.h>

int main(int argc, char **argv) {
    fsprint("disk scan in progress...\n");
    uint32_t sectors_count = c_ata_get_sectors_count();
    uint32_t used_sectors = c_fs_get_used_sectors(sectors_count);
    fsprint("$4total sector count: $1%d\n", sectors_count);
    fsprint("$4used sector count:  $1%d\n", used_sectors);
    return 0;
}
