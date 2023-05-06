#include <syscall.h>
#include <i_time.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char **argv) {
    char kver[32];
    c_sys_kinfo(kver);
    printf("$4kernel:     $1%s\n", kver);
    printf("$4FR time:    ");
    i_time_t time;
    c_time_get(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        itoa(time.full[i], tmp, 10);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        printf("$1%s$7:", tmp);
    }
    c_kprint_backspace(); printf(" ");
    for (int i = 3; i < 6; i++) {
        itoa(time.full[i], tmp, 10);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        printf("$1%s$7/", tmp);
    }
    int used_mem = c_mem_get_info(6, 0) / 1024;
    c_kprint_backspace();
    printf("$4\nunix time:  $1%d\n", time_gen_unix());
    printf("$4work time:  $1%d.%ds\n", (int) (c_timer_get_ms() / 1000), (int) (c_timer_get_ms() % 1000));
    printf("$4used mem:   $1%d.%dMo\n", used_mem / 1024, used_mem % 1024);
    printf("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_info(4, 0) - c_mem_get_info(5, 0), c_mem_get_info(4, 0));
    printf("$4phys mem:   $1%gMo\n", ((double) c_mem_get_info(0, 0) / 1024) / 1024);
    printf("$4disk size:  $1%gMo\n", ((double) c_fs_get_sector_count()) / 2048);
    printf("$4ramdisk:    $1%d%c $7($1%gMo$7)\n", (int)(100 * ((double) c_ramdisk_get_used()) / c_ramdisk_get_size()), '%', ((double) c_ramdisk_get_size()) / 2048);
    printf("$4screen:     $1%dx$1%d\n", c_vesa_get_width(), c_vesa_get_height());
    return 0;
}
