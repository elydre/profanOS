#include <syscall.h>
#include <i_string.h>
#include <i_iolib.h>
#include <i_time.h>

int main(int argc, char **argv) {
    fsprint("$4FR time:    ");
    time_t time;
    c_time_get(&time);
    time_jet_lag(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fsprint("$1%s$7:", tmp);
    }
    c_kprint_backspace(); fsprint(" ");
    for (int i = 3; i < 6; i++) {
        int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fsprint("$1%s$7/", tmp);
    }
    c_kprint_backspace();
    fsprint("$4\nunix time:  $1%d\n", time_gen_unix());
    fsprint("$4work time:  $1%dms\n", c_timer_get_tick());
    fsprint("$4used mem:   $1%dKo\n", c_mem_get_info(6, 0) / 1024);
    fsprint("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_info(4, 0) - c_mem_get_info(5, 0), c_mem_get_info(4, 0));
    fsprint("$4phys mem:   $1%fMo\n", ((double) c_mem_get_info(0, 0) / 1024) / 1024);
    fsprint("$4disk size:  $1%fMo\n", ((double) c_fs_get_sector_count()) / 2048);
    fsprint("$4ramdisk:    $1%d%c $7($1%fMo$7)\n", (int)(100 * ((double) c_ramdisk_get_used()) / c_ramdisk_get_size()), '%', ((double) c_ramdisk_get_size()) / 2048);
    fsprint("$4task alive: $1%d$7/$1%d\n", c_task_get_alive(), c_task_get_max());
    return 0;
}
