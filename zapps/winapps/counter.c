#include <i_winadds.h>
#include <stdlib.h>
#include <syscall.h>
#include <i_time.h>

int main(int argc, char **argv) {
    char tmp[4];

    c_process_wakeup(c_process_get_ppid(c_process_get_pid()));

    for (int i = 10; 1; i++) {
        if (i > 98) i = 10;

        itoa(i, tmp, 10);

        // add \n
        tmp[2] = '\n';
        tmp[3] = 0;

        wterm_append_string(tmp);
        ms_sleep(50);
    }
}
