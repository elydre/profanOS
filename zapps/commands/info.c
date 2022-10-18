#include <syscall.h>

int main(int argc, char **argv) {
    c_fskprint("$4FR time:    ");
    time_t time;
    c_time_get(&time);
    c_time_jet_lag(&time);
    char tmp[3];
    for (int i = 2; i >= 0; i--) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        c_fskprint("$1%s$7:", tmp);
    }
    c_kprint_backspace(); c_fskprint(" ");
    for (int i = 3; i < 6; i++) {
        c_int_to_ascii(time.full[i], tmp);
        if (tmp[1] == '\0') { tmp[1] = tmp[0]; tmp[0] = '0'; }
        c_fskprint("$1%s$7/", tmp);
    }
    c_kprint_backspace();
    c_fskprint("$4\nunix time:  $1%d\n", c_time_gen_unix());
    c_fskprint("$4ticks:      $1%d\n", c_timer_get_tick());
    c_fskprint("$4work time:  $1%ds$7/$1%ds\n", c_time_gen_unix() - c_time_get_boot() - c_timer_get_tick() / 100, c_time_gen_unix() - c_time_get_boot());
    c_fskprint("$4used mem:   $1%d%c\n", 100 * c_mem_get_usage() / c_mem_get_usable(), '%');
    c_fskprint("$4act alloc:  $1%d$7/$1%d\n", c_mem_get_alloc_count() - c_mem_get_free_count(), c_mem_get_alloc_count());
    c_fskprint("$4disk size:  $1%fMo\n", ((double) c_ata_get_sectors_count()) / 2048);
    c_task_print();
    return 0;
}