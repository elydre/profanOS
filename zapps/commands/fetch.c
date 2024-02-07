#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>

#define pl_and_pf(...) print_logo_line(); printf(__VA_ARGS__)

#define LOGO_LINES 9
#define INFO_LINES 9

#define LOGO_START_LINE ((INFO_LINES - LOGO_LINES) / 2)

char *LOGO[] = {
    "       \e[91m*\e[95m*\e[91m*\e[95m*\e[91m*     ",
    "     \e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m   ",
    "    \e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m*\e[95m*\e[91m  ",
    "    \e[91m*\e[95m*\e[91m*\e[95m*\e[0m/|\\\e[95m*\e[91m*\e[95m*\e[91m*  ",
    "     \e[91m*\e[95m*\e[0m//|\\\\\e[95m*\e[91m*   ",
    "      \e[0m///|\\\\\\\e[91m    ",
    "     \e[0m////|\\\\\\\\\e[91m   ",
    "\e[36m####\e[0m///\e[36m#####\e[0m\\\\\\  ",
    "   \e[96m#\e[36m####   \e[96m#\e[36m##\e[0m\\\\ ",
};

int curent_line;
int logo_line;

void print_logo_line(void) {
    // chek if we need to print a new line
    if (LOGO_START_LINE + logo_line > curent_line || LOGO_START_LINE + LOGO_LINES <= curent_line) {
        printf("                       ");
    } else {
        printf("%s      ", LOGO[logo_line++]);
    }

    curent_line++;
}

void print_fs_info(void) {
    print_logo_line();
    printf("\e[95mfs vdisk:   \e[96m");

    uint32_t *info = fu_get_vdisk_info();

    uint32_t dcount = info[0];
    for (uint32_t i = 0; i < dcount; i++) {
        printf("d%d: %dk", info[i*3 + 1], info[i*3 + 2] * FS_SECTOR_SIZE / 1024);
        if (i != dcount - 1) printf("\e[0m, \e[96m");
    }
    printf("\e[0m\n");

    free(info);
}

int main(void) {
    printf("\n \e[96m-\e[36m Welcome to the profan Operating System \e[96m-\n\n");

    curent_line = 0;
    logo_line = 0;

    tm_t time;
    c_time_get(&time);

    pl_and_pf("\e[95mkernel:     \e[96m%s\n", c_sys_kinfo());

    pl_and_pf("\e[95mRTC time:   \e[96m%02d\e[0m:\e[96m%02d\e[0m:\e[96m%02d %02d\e[0m/\e[96m%02d\e[0m/\e[96m%02d\n",
        time.tm_hour, time.tm_min, time.tm_sec,
        time.tm_mday, time.tm_mon, time.tm_year + 2000
    );
    print_fs_info();

    pl_and_pf("\e[95mwork time:  \e[96m%gs\n", c_timer_get_ms() / 1000.0);
    pl_and_pf("\e[95mused mem:   \e[96m%dkB\n", c_mem_get_info(6, 0) / 1024);

    pl_and_pf("\e[95mact alloc:  \e[96m%d\e[0m/\e[96m%d\n",
        c_mem_get_info(4, 0) - c_mem_get_info(5, 0),
        c_mem_get_info(4, 0)
    );

    pl_and_pf("\e[95mphys mem:   \e[96m%gMo\n", ((double) c_mem_get_info(0, 0) / 1024) / 1024);

    pl_and_pf("\e[95mscreen:     \e[96m%d\e[0mx\e[96m%d (%s)\n",
        c_vesa_get_width(), c_vesa_get_height(),
        c_vesa_does_enable() ? "graphic" : "text"
    );

    pl_and_pf("\n");

    printf("\e[0m\n");

    return 0;
}
