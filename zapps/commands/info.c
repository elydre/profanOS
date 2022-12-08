#include <syscall.h>
#include <iolib.h>

int main(int argc, char **argv) {
    fskprint("$4FR time:    ");
    time_t time;
    c_time_get(&time);
    c_time_jet_lag(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fskprint("$1%s$7:", tmp);
    }
    c_kprint_backspace(); fskprint(" ");
    for (int i = 3; i < 6; i++) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        fskprint("$1%s$7/", tmp);
    }
    c_kprint_backspace();
    fskprint("$4\nunix time:  $1%d\n", c_time_gen_unix());
    fskprint("$4ticks:      $1%d\n", c_timer_get_tick());
    fskprint("$4work time:  $1%ds$7/$1%ds\n", c_time_gen_unix() - c_time_get_boot() - c_timer_get_tick() / 1000, c_time_gen_unix() - c_time_get_boot());
    fskprint("$4used mem:   $1%d%c\n", 100 * c_mem_get_usage() / c_mem_get_usable(), '%');
    fskprint("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_alloc_count() - c_mem_get_free_count(), c_mem_get_alloc_count());
    fskprint("$4phys mem:   $1%fMo\n", ((double) c_mem_get_phys_size() / 1024) / 1024);
    fskprint("$4disk size:  $1%fMo\n", ((double) c_ata_get_sectors_count()) / 2048);
    fskprint("$4ramdisk:    $1%d%c $7($1%fMo$7)\n", (int)(100 * ((double) c_ramdisk_get_used()) / c_ramdisk_get_size()), '%', ((double) c_ramdisk_get_size()) / 2048);
    fskprint("$4task alive: $1%d$7/$1%d\n", c_task_get_alive(), c_task_get_max());
    return 0;
}
