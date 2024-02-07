#include <syscall.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

#define PROCESS_MAX 20

void str_insert(char *str, char *insert, int pos) {
    int len = strlen(str);
    int ilen = strlen(insert);
    if (pos > len)
        pos = len;
    if (pos < 0)
        pos = 0;
    memmove(str + pos + ilen, str + pos, len - pos + 1);
    memcpy(str + pos, insert, ilen);
}

void draw_line(char *buf, int percent) {
    int tmp;

    if (percent > 100)
        percent = 100;

    tmp = strlen(buf);
    memmove(buf + 51 - tmp, buf, tmp);
    memset(buf, ' ', 51 - tmp);
    buf[0] = '[';
    for (int i = 0; i < 50; i++) {
        if (i > percent / 2 || buf[i + 1] != ' ')
            break;
        buf[i + 1] = '|';
    }
    buf[51] = ']';
    buf[52] = '\0';

    // add colors
    str_insert(buf, "\e[0m", (percent == 100 ? 99: percent) / 2 + 2);
    str_insert(buf, "\e[31m", 1);
}

char *get_state(int state) {
    switch (state) {
        case 0: return "RUNNING";
        case 1: return "WAITING";
        case 2: return "TSLEEP";
        case 3: return "FSLEEP";
        case 4: return "ZOMBIE";
        case 5: return "DEAD";
        case 6: return "IDLE";
        default: return "UNKNOWN";
    }
}

void sort_tab(uint32_t *tab, int size) {
    uint32_t tmp;
    for (int i = 0; i < size; i++) {
        for (int j = i; j > 0 && tab[j] < tab[j - 1]; j--) {
            tmp = tab[j];
            tab[j] = tab[j - 1];
            tab[j - 1] = tmp;
        }
    }
}

void list_process(void) {
    uint32_t pid_list[PROCESS_MAX]; // it's a define
    char info_line[81];
    int pid_list_len = c_process_generate_pid_list(pid_list, PROCESS_MAX);
    sort_tab(pid_list, pid_list_len);

    int line_len = snprintf(info_line, 81, "  profanOS . kernel %s . %d processes . uptime %ds (%ds IDLE)",
            c_sys_kinfo(),
            pid_list_len - 2,
            c_timer_get_ms() / 1000,
            c_process_get_run_time(1) / 1000
    );

    // center info line
    int spaces = (79 - line_len) / 2;
    memmove(info_line + spaces, info_line, line_len);
    memset(info_line, ' ', spaces);
    info_line[80] = '\0';
    puts(info_line);

    puts("\e[47m\e[30m PID PPID     STATE     TIME      MEM ALLOC NAME                                \e[0m");
    int pid;
    char name[64];
    for (int i = 0; i < pid_list_len; i++) {
        pid = pid_list[i];
        if (pid == 1) continue;
        c_process_get_name(pid, name);
        printf("%4d %4d %9s %7gs %7dK %5d %-36s\n",
                pid,
                c_process_get_ppid(pid),
                get_state(c_process_get_state(pid)),
                c_process_get_run_time(pid) / 1000.0,
                c_mem_get_info(8, pid) / 1024,
                c_mem_get_info(7, pid),
                name
        );
    }
    printf("%80s\n", "");
}

int check_arg(int argc, char **argv) {
    if (argc > 2 || (argc == 2 && (argv[1][0] < '0' || argv[1][0] > '9'))) {
        printf("Usage: %s [sleep]\n", argv[0]);
        exit(1);
    }
    return argc == 2 ? atoi(argv[1]) : 1;
}

int main(int argc, char **argv) {
    int total_mem, used_mem, cpu, to_sleep;
    int last_idle, idle, last_total, total;
    char buf[64];

    to_sleep = check_arg(argc, argv);

    idle = total = 0;

    // clear screen
    printf("\e[2J");

    while (1) {
        // move cursor to top left
        printf("\e[H");

        last_idle = idle;
        last_total = total;

        idle = c_process_get_run_time(1);
        total = c_timer_get_ms();

        cpu = total - last_total;
        cpu = 100 - (idle - last_idle) * 100 / (cpu ? cpu : 1);
        if (cpu > 100)
            cpu = 100;

        tm_t t;
        c_time_get(&t);

        total_mem = c_mem_get_info(0, 0);
        used_mem = c_mem_get_info(6, 0);

        sprintf(buf, "%d%%", cpu);
        draw_line(buf, cpu);
        printf("%s %02d:%02d:%02d UTC\n", buf, t.tm_hour, t.tm_min, t.tm_sec);

        sprintf(buf, "%dM/%dM", used_mem / 1024 / 1024, total_mem / 1024 / 1024);
        draw_line(buf, used_mem * 100 / total_mem);
        printf("%s %d allocs    \n\n", buf, c_mem_get_info(4, 0) - c_mem_get_info(5, 0));
        list_process();
        sleep(to_sleep);
    }
    return 0;
}
