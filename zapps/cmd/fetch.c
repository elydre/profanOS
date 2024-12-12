/*****************************************************************************\
|   === fetch.c : 2024 ===                                                    |
|                                                                             |
|    Display system information with colors (like neofetch)        .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan.h>

#include <stdlib.h>
#include <stdio.h>

#define pl_and_pf(...) print_logo_line(); printf(__VA_ARGS__)

#define LOGO_LINES 9
#define INFO_LINES 11

#define FULL_CHAR 219

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
    printf("\e[95mfs vdisk   \e[96m");

    uint32_t *info = fu_get_vdisk_info();

    uint32_t dcount = info[0];
    for (uint32_t i = 0; i < dcount; i++) {
        printf("d%d: %dk", info[i*3 + 1], info[i*3 + 2] / 2);
        if (i != dcount - 1) printf("\e[0m, \e[96m");
    }
    printf("\e[0m\n");

    profan_kfree(info);
}

char *gen_all_colors(int base_color) {
    char *ret = malloc(1000);
    char *ptr = ret;
    for (int i = 0; i < 8; i++) {
        ptr += sprintf(ptr, "\e[%d%dm%c%c%c", base_color, i, FULL_CHAR, FULL_CHAR, FULL_CHAR);
    }
    return ret;
}

int main(void) {
    printf("\n \e[96m-\e[36m Welcome to the profan Operating System \e[96m-\n\n");

    curent_line = 0;
    logo_line = 0;

    tm_t time;
    syscall_time_get(&time);

    pl_and_pf("\e[95mkernel     \e[96m%s\n", syscall_sys_kinfo());
    pl_and_pf("\e[95mlibc       \e[96mprofan v%s\n", profan_libc_version());

    pl_and_pf("\e[95mRTC time   \e[96m%02d\e[0m:\e[96m%02d\e[0m:\e[96m%02d %02d\e[0m/\e[96m%02d\e[0m/\e[96m%02d\n",
        time.tm_hour, time.tm_min, time.tm_sec,
        time.tm_mday, time.tm_mon, time.tm_year + 2000
    );
    print_fs_info();

    int mem_usage = (uint32_t) syscall_mem_info(6, 0) / 1024;
    int mem_total = (uint32_t) syscall_mem_info(0, 0) / 1024;

    pl_and_pf("\e[95mwork time  \e[96m%gs of %gs\n",
            (syscall_timer_get_ms() - syscall_process_info(1, 3, NULL)) / 1000.0, syscall_timer_get_ms() / 1000.0);

    pl_and_pf("\e[95mmemory     \e[96m%.2f%% of %dMB\n",
            (float) mem_usage * 100 / (float) mem_total, mem_total / 1024);

    pl_and_pf("\e[95mscreen     \e[96m%d\e[0mx\e[96m%d (%s)\n",
        syscall_vesa_width(), syscall_vesa_height(),
        syscall_vesa_state() ? "graphic" : "text"
    );

    pl_and_pf("\n");

    char *all_colors;

    all_colors = gen_all_colors(3);
    pl_and_pf("%s\n", all_colors);
    free(all_colors);
    all_colors = gen_all_colors(9);
    pl_and_pf("%s\n", all_colors);
    free(all_colors);

    printf("\e[0m\n");

    return 0;
}
