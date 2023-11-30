#include <stdlib.h>
#include <stdio.h>

#include <syscall.h>
#include <filesys.h>

#define pl_and_pf(...) print_logo_line(); printf(__VA_ARGS__)

#define LOGO_LINES 9
#define INFO_LINES 9

#define LOGO_START_LINE ((INFO_LINES - LOGO_LINES) / 2)

char *LOGO[] = {
    "       \033[91m*\033[95m*\033[91m*\033[95m*\033[91m*     ",
    "     \033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m   ",
    "    \033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m*\033[95m*\033[91m  ",
    "    \033[91m*\033[95m*\033[91m*\033[95m*\033[0m/|\\\033[95m*\033[91m*\033[95m*\033[91m*  ",
    "     \033[91m*\033[95m*\033[0m//|\\\\\033[95m*\033[91m*   ",
    "      \033[0m///|\\\\\\\033[91m    ",
    "     \033[0m////|\\\\\\\\\033[91m   ",
    "\033[36m####\033[0m///\033[36m#####\033[0m\\\\\\  ",
    "   \033[96m#\033[36m####   \033[96m#\033[36m##\033[0m\\\\ ",
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
    printf("\033[95mfs vdisk:   \033[96m");

    uint32_t *info = fu_get_vdisk_info();

    uint32_t dcount = info[0];
    for (uint32_t i = 0; i < dcount; i++) {
        printf("d%d: %dk", info[i*3 + 1], info[i*3 + 2] * FS_SECTOR_SIZE / 1024);
        if (i != dcount - 1) printf("\033[0m, \033[96m");
    }
    printf("\033[0m\n");

    free(info);
}

int main(void) {
    printf("\n \033[96m-\033[36m Welcome to the profan Operating System \033[96m-\n\n");

    curent_line = 0;
    logo_line = 0;

    tm_t time;
    c_time_get(&time);

    pl_and_pf("\033[95mkernel:     \033[96m%s\n", c_sys_kinfo());

    pl_and_pf("\033[95mRTC time:   \033[96m%02d\033[0m:\033[96m%02d\033[0m:\033[96m%02d %02d\033[0m/\033[96m%02d\033[0m/\033[96m%02d\n",
        time.tm_hour, time.tm_min, time.tm_sec,
        time.tm_mday, time.tm_mon + 1, time.tm_year + 2000
    );
    print_fs_info();

    pl_and_pf("\033[95mwork time:  \033[96m%gs\n", c_timer_get_ms() / 1000.0);
    pl_and_pf("\033[95mused mem:   \033[96m%dkB\n", c_mem_get_info(6, 0) / 1024);

    pl_and_pf("\033[95mact alloc:  \033[96m%d\033[0m/\033[96m%d\n",
        c_mem_get_info(4, 0) - c_mem_get_info(5, 0),
        c_mem_get_info(4, 0)
    );

    pl_and_pf("\033[95mphys mem:   \033[96m%gMo\n", ((double) c_mem_get_info(0, 0) / 1024) / 1024);

    pl_and_pf("\033[95mscreen:     \033[96m%d\033[0mx\033[96m%d (%s)\n",
        c_vesa_get_width(), c_vesa_get_height(),
        c_vesa_does_enable() ? "text" : "graphics"
    );

    pl_and_pf("\n");

    printf("\033[0m\n");

    return 0;
}
