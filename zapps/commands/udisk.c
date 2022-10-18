#include <syscall.h>

int main(int argc, char **argv) {
    c_fskprint("disk scan in progress...\n");
    uint32_t sectors_count = c_ata_get_sectors_count();
    uint32_t used_sectors = c_fs_get_used_sectors(sectors_count);
    c_fskprint("$4total sector count: $1%d\n", sectors_count);
    c_fskprint("$4used sector count:  $1%d\n", used_sectors);
    return 0;
}