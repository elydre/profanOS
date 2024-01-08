#include <syscall.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <time.h>

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
    str_insert(buf, "\033[0m", (percent == 100 ? 99: percent) / 2 + 2);
    str_insert(buf, "\033[31m", 1);
}


int main(void) {
    int total_mem, used_mem, cpu;
    int last_idle, idle, last_total, total;
    char buf[64];

    idle = total = 0;

    // clear screen
    printf("\033[2J");

    while (1) {
        // move cursor to top left
        printf("\033[H");

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
        printf("%s %d allocs\n", buf, c_mem_get_info(4, 0) - c_mem_get_info(5, 0));

        fflush(stdout);
        sleep(1);
    }
    return 0;
}
