#include <syscall.h>
#include <i_time.h>
#include <stdlib.h>
#include <stdio.h>

#define pl_and_pf(...) print_logo_line(); printf(__VA_ARGS__)

#define LOGO_LINES 9
#define INFO_LINES 9

#define LOGO_START_LINE ((INFO_LINES - LOGO_LINES) / 2)

char *LOGO[] = {
    "       $3*$4*$3*$4*$3*     ",
    "     $4*$3*$4*$3*$4*$3*$4*$3*$4*$3   ",
    "    $4*$3*$4*$3*$4*$3*$4*$3*$4*$3*$4*$3  ",
    "    $3*$4*$3*$4*$7/|\\$4*$3*$4*$3*  ",
    "     $3*$4*$7//|\\\\$4*$3*   ",
    "      $7///|\\\\\\$3    ",
    "     $7////|\\\\\\\\$3   ",
    "$A####$7///$A#####$7\\\\\\  ",
    "   $2#$A####   $2#$A##$7\\\\ ",
};

int curent_line;
int logo_line;

void print_logo_line() {
    // chek if we need to print a new line
    if (LOGO_START_LINE + logo_line > curent_line || LOGO_START_LINE + LOGO_LINES <= curent_line) {
        printf("                       ");
    } else {
        printf("%s      ", LOGO[logo_line++]);
    }

    curent_line++;
}

int main(void) {
    printf("\n $2-$A Welcome to the profan Operating System $2-\n\n");

    curent_line = 0;
    logo_line = 0;

    char kver[32];
    c_sys_kinfo(kver);

    i_time_t time;
    c_time_get(&time);

    int used_mem = c_mem_get_info(6, 0) / 1024;

    pl_and_pf("$4kernel:     $2%s\n", kver);

    pl_and_pf("$4UTC time:   $2%02d$7:$2%02d$7:$2%02d %02d$7/$2%02d$7/$2%02d\n",
        time.hours, time.minutes, time.seconds,
        time.day_of_month, time.month, time.year
    );

    pl_and_pf("$4work time:  $2%gs\n", c_timer_get_ms() / 1000.0);
    pl_and_pf("$4used mem:   $2%d.%dMo\n", used_mem / 1024, used_mem % 1024);

    pl_and_pf("$4act alloc:  $2%d$7/$2%d\n",
        c_mem_get_info(4, 0) - c_mem_get_info(5, 0),
        c_mem_get_info(4, 0)
    );

    pl_and_pf("$4phys mem:   $2%gMo\n", ((double) c_mem_get_info(0, 0) / 1024) / 1024);

    pl_and_pf("$4screen:     $2%dx$2%d\n", c_vesa_get_width(), c_vesa_get_height());
    pl_and_pf("\n");
    pl_and_pf("\n");

    printf("\n");

    return 0;
}
