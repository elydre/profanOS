#include <syscall.h>
#include <string.h>
#include <iolib.h>

int main(int argc, char **argv) {
    fsprint("$4FR time:    ");
    time_t time;
    c_time_get(&time);
    c_time_jet_lag(&time);
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
    fsprint("$4\nunix time:  $1%d\n", c_time_gen_unix());
    fsprint("$4ticks:      $1%d\n", c_timer_get_tick());
    fsprint("$4work time:  $1%ds$7/$1%ds\n", c_time_gen_unix() - c_time_get_boot() - c_timer_get_tick() / 1000, c_time_gen_unix() - c_time_get_boot());
    fsprint("$4used mem:   $1%d%c\n", 100 * c_mem_get_usage() / c_mem_get_usable(), '%');
    fsprint("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_alloc_count() - c_mem_get_free_count(), c_mem_get_alloc_count());
    fsprint("$4phys mem:   $1%fMo\n", ((double) c_mem_get_phys_size() / 1024) / 1024);
    fsprint("$4disk size:  $1%fMo\n", ((double) c_ata_get_sectors_count()) / 2048);
    fsprint("$4ramdisk:    $1%d%c $7($1%fMo$7)\n", (int)(100 * ((double) c_ramdisk_get_used()) / c_ramdisk_get_size()), '%', ((double) c_ramdisk_get_size()) / 2048);
    fsprint("$4task alive: $1%d$7/$1%d\n", c_task_get_alive(), c_task_get_max());
    return 0;
}
