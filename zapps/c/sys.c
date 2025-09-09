/*****************************************************************************\
|   === sys.c : 2024 ===                                                      |
|                                                                             |
|    Command to display memory usage                               .pi0iq.    |
|                                                                 d"  . `'b   |
|    This file is part of profanOS and is released under          q. /|\  "   |
|    the terms of the GNU General Public License                   `// \\     |
|                                                                  //   \\    |
|   === elydre : https://github.com/elydre/profanOS ===         #######  \\   |
\*****************************************************************************/

// @LINK: libpf

#include <profan/syscall.h>
#include <profan/filesys.h>
#include <profan/carp.h>
#include <profan.h>

#include <sys/utsname.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

#define PROCESS_MAX 200

#define LOGO_LINES 9
#define INFO_LINES 11

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
    for (int i = 0; i < 8; i++)
        ptr += sprintf(ptr, "\e[%d%dm   ", base_color, i);
    ptr += sprintf(ptr, "\e[0m");
    return ret;
}

#define pl_and_pf(...) (print_logo_line(), printf(__VA_ARGS__))

int do_fetch(void) {
    printf("\n \e[96m-\e[36m Welcome to the profan Operating System \e[96m-\n\n");

    curent_line = 0;
    logo_line = 0;

    struct tm time;
    syscall_time_get(&time);

    struct utsname name;
    uname(&name);

    pl_and_pf("\e[95mkernel     \e[96m%s %s\n", name.version, name.release);
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

    all_colors = gen_all_colors(4);
    pl_and_pf("%s\n", all_colors);
    free(all_colors);
    all_colors = gen_all_colors(10);
    pl_and_pf("%s\n", all_colors);
    free(all_colors);

    pl_and_pf("\n");

    return 0;
}

void sort_tab(uint32_t *tab, int size) {
    int tmp;
    for (int i = 0; i < size; i++) {
        for (int j = i; j > 0 && tab[j] < tab[j - 1]; j--) {
            tmp = tab[j];
            tab[j] = tab[j - 1];
            tab[j - 1] = tmp;
        }
    }
}

void memory_print_usage(void) {
    int alloc_count = syscall_mem_info(4, 0);
    int free_count = syscall_mem_info(5, 0);

    puts("      -------- General --------");

    printf("Total memory      %12d MB\n", syscall_mem_info(0, 0) / 1024 / 1024);
    printf("AT alloc count    %15d\n", alloc_count);
    printf("AT free count     %15d\n", free_count);
    printf("base address      %15p\n", syscall_mem_info(1, 0));
    printf("MEM_PARTS addr    %15p\n", syscall_mem_info(3, 0));
    printf("MEM_PARTS size    %12d B  | 1\n", syscall_mem_info(2, 0));
    printf("Used memory       %12d kB | %d\n", syscall_mem_info(6, 0) / 1024, alloc_count - free_count);

    puts("\n      ------- Per types -------");

    printf("Simple alloc   %15d kB | %d\n", syscall_mem_info(10, 1) / 1024, syscall_mem_info(9, 1));
    printf("Scuba vpage    %15d kB | %d\n", syscall_mem_info(10, 3) / 1024, syscall_mem_info(9, 3));
    printf("Module Loader  %15d kB | %d\n", syscall_mem_info(10, 4) / 1024, syscall_mem_info(9, 4));
    printf("As kernel      %15d kB | %d\n", syscall_mem_info(10, 2) / 1024, syscall_mem_info(9, 2));

    puts("\n      ------ Per process ------");

    uint32_t pid_list[PROCESS_MAX]; // it's a define
    int pid_list_len = syscall_process_list_all(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    int pid;
    char *name;
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (pid == 1) continue;
        name = (char *) syscall_process_info(pid, PROC_INFO_NAME, NULL);
        printf("PID %-3.02d %-15s %6d kB | %-3d (%d)\n",
                pid, (strchr(name, '/') ? strrchr(name, '/') + 1 : name),
                syscall_mem_info(12, pid) / 1024,
                syscall_mem_info(11, pid),
                syscall_mem_info(7, pid)
        );
    }
    putchar('\n');
}

void memory_print_summary(void) {
    int current = syscall_mem_info(6, 0);
    int total = syscall_mem_info(0, 0);
    printf("About %.2f %% of memory is currently used,\n", (float) current / (float) total * 100);
    printf("In total, %d MB can be allocated!\n", total / 1024 / 1024);
}

int main(int argc, char **argv) {
    carp_init("[options]", 0);

    carp_register('m', CARP_STANDARD, "show detailed memory usage");
    carp_register('s', CARP_STANDARD, "show summary of resources");

    carp_conflict("ms");

    if (carp_parse(argc, argv))
        return 1;

    if (carp_isset('m'))
        memory_print_usage();
    else if (carp_isset('s'))
        memory_print_summary();
    else
        do_fetch();

    return 0;
}
